#include "CGI.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>

CGI::CGI() {}
CGI::~CGI() {}

enum	CgiType
{
	BINARY,
	PYTHON,
	PERL,
	PHP,
	SHELL,
	JS,
	HTML,
	CSS,
	UNKNOWN
};

std::string CGI::_getExtension(const std::string &path) const
{
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return "";
	return path.substr(pos); // includes the dot: ".ext"
}

int CGI::_getCgiType(const std::string &ext) const
{
	if (ext == ".cgi")
		return BINARY;
	if (ext == ".py") 
		return PYTHON;
	if (ext == "pl")
		return PERL;
	if (ext == ".php")
		return PHP;
	if (ext == ".sh")
		return SHELL;
	if (ext == ".js")
		return JS;
	if (ext == ".html")
		return HTML;
	if (ext == ".css")
		return CSS;
	return UNKNOWN;
}

std::vector<std::string> CGI::_split(const std::string &s) const
{
	std::istringstream iss(s);
	std::vector<std::string> result;
	std::string word;
	while (iss >> word)
		result.push_back(word);
	return result;
}

//------------------------------------ CGI ENV BUILDING ------------------------------------//

std::vector<std::string> CGI::_buildCgiEnv(const HttpRequest &req, const Server &server, const std::string &scriptPath) const
{
	std::vector<std::string> env;
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");

	std::ostringstream oss;
	oss << "SERVER_PORT=" << (server.getPorts().empty() ? 80 : server.getPorts()[0]);//wrong port, need to use our MAPS in LaunchServer
	env.push_back(oss.str());
	env.push_back("SERVER_NAME=" + server.getName());
	env.push_back("REQUEST_METHOD=" + req.method);
	env.push_back("SCRIPT_FILENAME=" + scriptPath);
	env.push_back("QUERY_STRING=" + (req.header.count("Query") ? req.header.find("Query")->second : ""));
	env.push_back("CONTENT_TYPE=" + (req.header.count("Content-Type") ? req.header.find("Content-Type")->second : ""));
	env.push_back("CONTENT_LENGTH=" + (req.header.count("Content-Length") ? req.header.find("Content-Length")->second : ""));
	env.push_back("REDIRECT_STATUS=200"); // required for php-cgi

	for (std::map<std::string, std::string>::const_iterator it = req.header.begin(); it != req.header.end(); it++)
	{
		std::string key = it->first;
		for (size_t i = 0; i < key.size(); ++i)
		{
			if (key[i] == '-')
				key[i] = '_';
			else
				key[i] = toupper(key[i]);
		}
		env.push_back("HTTP_" + key + "=" + it->second);
	}
	return env;
}

std::vector<char*> CGI::_envVecToCharPtr(const std::vector<std::string> &env) const
{
	std::vector<char*> vec;
	for (size_t i = 0; i < env.size(); i++)
		vec.push_back(const_cast<char*>(env[i].c_str()));
	vec.push_back(NULL);
	return vec;
}

void CGI::freeEnvCharVec(std::vector<char*> &vec) const
{
	(void)vec; // Nothing to free
}

std::string CGI::_readFromFd(int fd) const
{
	std::string result;
	char buf[4096];
	ssize_t n;
	while ((n = read(fd, buf, sizeof(buf))) > 0)
		result.append(buf, n);
	return result;
}

std::string CGI::_parseCgiOutput(const std::string &raw, int &outStatusCode, std::map<std::string,std::string> &outHeaders) const
{
	std::string::size_type headerEnd = raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		headerEnd = raw.find("\n\n");

	std::string headers = raw.substr(0, headerEnd);
	std::string body = (headerEnd == std::string::npos) ? "" : raw.substr(headerEnd + 4);

	std::istringstream iss(headers);
	std::string line;
	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.size()-1] == '\r')
			line.erase(line.size()-1);
		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);
			outHeaders[key] = value;
		}
	}
	outStatusCode = (outHeaders.find("Status") != outHeaders.end())
		? atoi(outHeaders["Status"].c_str())
		: 200;
	return body;
}

//------------------------------------ CGI INTERPRETER ------------------------------------//

/**
 * Automatically maps CGI extensions to interpreters from the config.
 * Example:
 *   cgi_ext  = ".php .py"
 *   cgi_path = "/usr/bin/php-cgi /usr/bin/python3"
 */

int CGI::_checkAccess(const std::string &path, int type)
{
	if (access(path.c_str(), F_OK) == -1)
		return (-1);
	if (type == BINARY && access(path.c_str(), X_OK) == -1)
		return (0);
	if (access(path.c_str(), R_OK) == -1)
		return (0);
	return (1);
}

std::string CGI::getCgiInterpreter(const std::string &ext, const Server &server) const
{
	std::map<std::string, std::string> data = server.getData();
	std::map<std::string, std::string>::const_iterator itExt = data.find("cgi_ext");
	std::map<std::string, std::string>::const_iterator itPath = data.find("cgi_path");
	if (itExt == data.end() || itPath == data.end())
	{
		// fallback defaults
		if (ext == ".php")
			return "/usr/bin/php-cgi";
		if (ext == ".py")
			return "/usr/bin/python3";
		return "";
	}

	std::vector<std::string> exts = _split(itExt->second);
	std::vector<std::string> paths = _split(itPath->second);
	for (size_t i = 0; i < exts.size() && i < paths.size(); ++i)
	{
		if (exts[i] == ext)
			return paths[i];
	}

	// fallback if not found
	if (ext == ".php")
		return "/usr/bin/php-cgi";
	if (ext == ".py") 
		return "/usr/bin/python3";
	return "";
}

//------------------------------------ EXECUTE CGI ------------------------------------//

std::string CGI::executeCgi(const HttpRequest &request, const Server &server)
{
	std::string	ext = _getExtension(request.path);
	std::string	interpreter = getCgiInterpreter(ext, server);
	int	type = _getCgiType(ext);

		// --- 1. Check access ---
	if (_checkAccess(request.path, type) <= 0)
		throw std::runtime_error("CGI file not accessible: " + request.path);

	if (interpreter.empty() || type == UNKNOWN)
		throw std::runtime_error("Unsupported CGI extension: " + ext);

	std::vector<std::string> cgiEnv = _buildCgiEnv(request, server, request.path);
	std::vector<char*> envp = _envVecToCharPtr(cgiEnv);

	int pipeIn[2];
	int pipeOut[2];
	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		throw std::runtime_error("pipe() failed: " + std::string(strerror(errno)));

	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("fork() failed: " + std::string(strerror(errno)));

	if (pid == 0)
	{
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[1]);
		close(pipeOut[0]);

		char *argv[3];
		argv[0] = const_cast<char*>(interpreter.c_str());
		argv[1] = const_cast<char*>(request.path.c_str());
		argv[2] = NULL;

		execve(interpreter.c_str(), argv, &envp[0]);
		std::cerr << "execve failed: " << strerror(errno) << std::endl;
		_exit(1);
	}

	close(pipeIn[0]); // Close read end of stdin pipe
	close(pipeOut[1]); // Close write end of stdout pipe

	// Write POST body if present
	if (request.method == "POST")
	{
		std::string body;
		for (std::map<std::string, std::string>::const_iterator it = request.body.begin();
			 it != request.body.end(); it++)
			body += it->first + "=" + it->second + "&";
		if (!body.empty())
			body.erase(body.size() - 1);
		write(pipeIn[1], body.c_str(), body.size());
	}
	// Close write end of pipeIn so the child sees EOF and terminates
	close(pipeIn[1]);

	// Read all CGI output until EOF
	std::string rawOutput = _readFromFd(pipeOut[0]);
	close(pipeOut[0]);

	// Wait for CGI child to exit
	int status = 0;
	waitpid(pid, &status, 0);

	int cgiStatus = 200;
	std::map<std::string,std::string> headers;
	std::string body = _parseCgiOutput(rawOutput, cgiStatus, headers);

	std::ostringstream response;
	response << "HTTP/1.1 " << cgiStatus << " OK\r\n";

	// Add Content-Length if CGI didn’t send one!!! Else: infinite select wait
	if (headers.find("Content-Length") == headers.end())
	{
		std::stringstream oss;
		oss << body.size();
		headers["Content-Length"] = oss.str();
	}

	for (std::map<std::string,std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n" << body;

	freeEnvCharVec(envp);
	return response.str();
}

// int	CGI::executeCgi(const HttpRequest &request, const Server &server)
// {
// 	std::string	ext = _getExtension(request.path);
// 	std::string	interpreter = getCgiInterpreter(ext, server);
// 	int	type = _getCgiType(ext);

// 	switch (_checkAccess(request.path, type))
// 	{
// 		case -1:
// 			throw std::runtime_error("No request cgi path access: " + ext);
// 			//throw CGIException("file " + request.path + " does not exist", false, 404, Server.getUid());
// 		case 0:
// 			throw std::runtime_error("Do not have permission to access : " + request.path);
// 			//throw CGIException("Do not have permission to access :" + request.path + " on this server", false, 403, Server.getUid());
// 		case 1:
// 			break;
// 	}

// 	if (interpreter.empty())
// 		throw std::runtime_error("Unsupported CGI extension: " + ext);
	
// 	if (type == UNKNOWN)
// 		throw std::runtime_error("Unknown CGI extension: " + ext);
// 		//throw CGIException("Webserver does not interpret file: " + path, false, 415, Server.getUid());

// 	std::vector<std::string> cgiEnv = _buildCgiEnv(request, server, request.path);
// 	std::vector<char*> envp = _envVecToCharPtr(cgiEnv);

// 	int pipeIn[2];
// 	int pipeOut[2];
// 	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
// 		throw std::runtime_error("pipe() failed: " + std::string(strerror(errno)));

// 	pid_t pid = fork();

// 	if (pid < 0)
// 		throw std::runtime_error("fork() failed: " + std::string(strerror(errno)));

// 	if (pid == 0)
// 	{
// 		dup2(pipeIn[0], STDIN_FILENO);
// 		dup2(pipeOut[1], STDOUT_FILENO);
// 		close(pipeIn[1]);
// 		close(pipeOut[0]);

// 		char *argv[3];
// 		argv[0] = const_cast<char*>(interpreter.c_str());
// 		argv[1] = const_cast<char*>(request.path.c_str());
// 		argv[2] = NULL;

// 		execve(interpreter.c_str(), argv, &envp[0]);
// 		std::cerr << "execve failed: " << strerror(errno) << std::endl;
// 		_exit(1);
// 	}

// 	close(pipeIn[0]);
// 	close(pipeOut[1]);

// 	if (request.method == "POST")
// 	{
// 		std::string body;
// 		for (std::map<std::string, std::string>::const_iterator it = request.body.begin();
// 			 it != request.body.end(); ++it)
// 			body += it->first + "=" + it->second + "&";
// 		if (!body.empty())
// 			body.erase(body.size()-1);
// 		write(pipeIn[1], body.c_str(), body.size());
// 	}
// 	close(pipeIn[1]);

// 	std::string rawOutput = _readFromFd(pipeOut[0]);
// 	close(pipeOut[0]);

// 	int status = 0;
// 	waitpid(pid, &status, 0);

// 	int cgiStatus = 200;
// 	std::map<std::string,std::string> headers;
// 	std::string body = _parseCgiOutput(rawOutput, cgiStatus, headers);

// 	std::ostringstream response;
// 	response << "HTTP/1.1 " << cgiStatus << " OK\r\n";
// 	for (std::map<std::string,std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
// 		response << it->first << ": " << it->second << "\r\n";
// 	response << "\r\n" << body;

// 	freeEnvCharVec(envp);

// 	return response.str();
// }
//need to modify process to directly send from CGI function, and not copy a result inside sendResponse()
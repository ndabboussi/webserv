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

// Extracts the file extension (including the dot) from a given path
// Ex: "/cgi-bin/hello.py" → ".py"
std::string CGI::_getExtension(const std::string &path) const
{
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return "";
	return path.substr(pos); // includes the dot: ".ext"
}

//Returns an enum representing the CGI type based on the file extension
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

// Splits a string into a list of words separated by whitespace.
// Used mainly for parsing configuration values (e.g., multiple CGI extensions)
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

// Builds the environment variables passed to the CGI script.
// Includes standard CGI variables (REQUEST_METHOD, QUERY_STRING, etc.)
// and also adds HTTP headers with the "HTTP_" prefix
std::vector<std::string> CGI::_buildCgiEnv(const HttpRequest &req, const Server &server, const std::string &scriptPath) const
{
	std::vector<std::string> env;
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");

	std::ostringstream oss;
	oss << "SERVER_PORT=" << req.serverPort;
	env.push_back(oss.str());
	env.push_back("SERVER_NAME=" + server.getName());
	env.push_back("REQUEST_METHOD=" + req.method);
	env.push_back("SCRIPT_FILENAME=" + scriptPath);
	env.push_back("QUERY_STRING=" + (req.header.count("Query") ? req.header.find("Query")->second : ""));
	env.push_back("CONTENT_TYPE=" + (req.header.count("Content-Type") ? req.header.find("Content-Type")->second : ""));
	env.push_back("CONTENT_LENGTH=" + (req.header.count("Content-Length") ? req.header.find("Content-Length")->second : ""));
	env.push_back("REDIRECT_STATUS=200"); // required for php-cgi

	// Transform headers into CGI environment format (HTTP_HEADER_NAME=value)
	for (std::map<std::string, std::string>::const_iterator it = req.header.begin(); it != req.header.end(); it++)
	{
		std::string key = it->first;
		for (size_t i = 0; i < key.size(); i++)
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

// Converts a vector of strings into a NULL-terminated array of char* suitable for passing to execve()
std::vector<char*> CGI::_envVecToCharPtr(const std::vector<std::string> &env) const
{
	std::vector<char*> vec;
	for (size_t i = 0; i < env.size(); i++)
		vec.push_back(const_cast<char*>(env[i].c_str()));
	vec.push_back(NULL);
	return vec;
}

// Reads all data from a file descriptor until EOF and returns it as a string
// Used to collect CGI output from the child process
std::string CGI::_readFromFd(int fd) const
{
	std::string result;
	char buf[4096];
	ssize_t n;
	while ((n = read(fd, buf, sizeof(buf))) > 0)
		result.append(buf, n);
	return result;
}

// Parses the raw CGI output into headers and body
// Extracts "Status" if present, otherwise defaults to 200
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
	//Default to 200 if CGI didn’t send a "Status" header
	outStatusCode = (outHeaders.find("Status") != outHeaders.end())
		? atoi(outHeaders["Status"].c_str()) : 200;
	return body;
}

//------------------------------------ CGI INTERPRETER ------------------------------------//

//Ensures that the target CGI file exists and has proper permissions
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

// Determines the interpreter to use for the given CGI extension
// Reads configuration values, or falls back to system defaults

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

// Main function that:
//   1. Sets up environment and pipes
//   2. Forks the process
//   3. Executes the CGI script via execve()
//   4. Reads CGI output
//   5. Builds a proper HTTP response
// Returns: A complete HTTP/1.1 response as a string
std::string CGI::executeCgi(const HttpRequest &request, const Server &server)
{
	std::string	ext = _getExtension(request.path);
	std::string	interpreter = getCgiInterpreter(ext, server);
	int	type = _getCgiType(ext);

	// 1. Check access and interpreter validity
	if (_checkAccess(request.path, type) <= 0)
		throw std::runtime_error("CGI file not accessible: " + request.path);

	if (interpreter.empty() || type == UNKNOWN)
		throw std::runtime_error("Unsupported CGI extension: " + ext);

	//2. Build environment and create pipes
	std::vector<std::string> cgiEnv = _buildCgiEnv(request, server, request.path);
	std::vector<char*> envp = _envVecToCharPtr(cgiEnv);

	int pipeIn[2];
	int pipeOut[2];
	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		throw std::runtime_error("pipe() failed: " + std::string(strerror(errno)));

	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("fork() failed: " + std::string(strerror(errno)));

	//3. Child process
	if (pid == 0)
	{
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		//Close unused ends to prevent deadlock
		close(pipeIn[1]);
		close(pipeOut[0]);
		//Prepare args for execve()
		char *argv[3];
		argv[0] = const_cast<char*>(interpreter.c_str());
		argv[1] = const_cast<char*>(request.path.c_str());
		argv[2] = NULL;
		// Execute CGI interpreter
		execve(interpreter.c_str(), argv, &envp[0]);
		std::cerr << "execve failed: " << strerror(errno) << std::endl;
		// If execve fails, print error and exit
		_exit(1);
	}

	//4. Parent process
	close(pipeIn[0]); // Close read end of stdin pipe
	close(pipeOut[1]); // Close write end of stdout pipe

	// If POST, send body to child's stdin
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
	
	close(pipeIn[1]); // Close write end of pipeIn so the child sees EOF and terminates

	// 5. Read all CGI output until EOF
	std::string rawOutput = _readFromFd(pipeOut[0]);
	close(pipeOut[0]);

	// Wait for CGI child to exit
	int status = 0;
	waitpid(pid, &status, 0);

	//6. Parse and construct HTTP response
	int cgiStatus = 200;
	std::map<std::string,std::string> headers;
	std::string body = _parseCgiOutput(rawOutput, cgiStatus, headers);

	std::ostringstream response;
	response << "HTTP/1.1 " << cgiStatus << " OK\r\n";

	// Add Content-Length if CGI didn’t send one!!! Important: Without Content-Length, browser never knows response end
	if (headers.find("Content-Length") == headers.end())
	{
		std::stringstream oss;
		oss << body.size();
		headers["Content-Length"] = oss.str();
	}

	for (std::map<std::string,std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n" << body;

	return response.str();
}

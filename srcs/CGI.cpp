#include "CGI.hpp"

//Constructor/Destructors--------------------------------------------------

// CGI::CGI()
// {}

CGI::CGI(HttpRequest &request) : _request(request)
{}

CGI::CGI(CGI const &src) : _request(src._request)
{
	*this = src;
	return ;
}

CGI &CGI::operator=(CGI const &src)
{
	if (this != &src)
	{
		_request = src._request;
		_extension = src._extension;
		_path = src._path;
		_interpreter = src._interpreter;
		_type = src._type;
	}
	return (*this);
}

CGI::~CGI(void)
{}

// Extracts the file extension (including the dot) from a given path
// Ex: "/cgi-bin/hello.py" → ".py"
std::string CGI::setExtension() const
{
	size_t pos = this->_path.find_last_of('.');
	if (pos == std::string::npos)
		return "";
	return this->_path.substr(pos); // includes the dot: ".ext"
}

//Returns an enum representing the CGI type based on the file extension
int CGI::getCgiType() const
{
	if (this->_extension == ".cgi")//meaning its already executable
		return BINARY;
	if (this->_extension == ".py") 
		return PYTHON;
	if (this->_extension == ".pl")
		return PERL;
	if (this->_extension == ".php")
		return PHP;
	if (this->_extension == ".sh")
		return SHELL;
	if (this->_extension == ".js")
		return JS;
	if (this->_extension == ".html")
		return HTML;
	if (this->_extension == ".css")
		return CSS;
	return UNKNOWN;
}

// Splits a string into a list of words separated by whitespace.
std::vector<std::string> CGI::_split(const std::string &str) const
{
	std::istringstream iss(str);
	std::vector<std::string> result;
	std::string word;
	while (iss >> word)
		result.push_back(word);
	return result;
}

void	CGI::setCgiInfos(const HttpRequest &request, const Server &server)
{
	this->_path = request.path;
	this->_extension = this->setExtension();
	this->_type = this->getCgiType();

	this->_defaults[".cgi"] = "";
	this->_defaults[".py"] = "/usr/bin/python3";
	this->_defaults[".pl"] = "/usr/bin/perl";
	this->_defaults[".php"] = "/usr/bin/php-cgi";
	this->_defaults[".sh"] = "/bin/bash";
	this->_defaults[".js"] = "/usr/bin/node";
	this->_defaults[".html"] = "";
	this->_defaults[".css"] = "";

	// Determines the interpreter to use for the given CGI extension
	// Reads configuration values, or falls back to system defaults
	std::map<std::string, std::string> data = server.getData();
	std::map<std::string, std::string>::const_iterator itExt = data.find("cgi_ext");
	std::map<std::string, std::string>::const_iterator itPath = data.find("cgi_path");
	if (itExt != data.end() && itPath != data.end())
	{
		std::vector<std::string> splittedExtensions = _split(itExt->second);
		std::vector<std::string> splittedPaths = _split(itPath->second);
		for (size_t i = 0; i < splittedExtensions.size() && i < splittedPaths.size(); i++)
		{
			if (splittedExtensions[i] == this->_extension)
			{
				this->_interpreter = splittedPaths[i];
				return;
			}
		}
	}
	if (this->_interpreter.empty() && this->_defaults.count(this->_extension))
		this->_interpreter = this->_defaults[this->_extension];
	return;
}

int CGI::checkAccess() const
{
	if (access(this->_path.c_str(), F_OK) == -1) // Check if the file exists
	{
		this->_request.statusCode = 404;
		throw std::runtime_error("[CGI ERROR] File not found: " + this->_path);
	}

	if (access(this->_path.c_str(), X_OK) == -1) // Check execute permission
	{
		this->_request.statusCode = 403;
		throw std::runtime_error("[CGI ERROR] File is not executable: " + this->_path);
	}
	if (access(this->_path.c_str(), R_OK) == -1) // Check read permission
	{
		this->_request.statusCode = 403;
		throw std::runtime_error("[CGI ERROR] File is not readable: " + this->_path);
	}
	return 1;
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

//------------------------------------ PREPARE CGI BODY ------------------------------------//

// Reads all data from a file descriptor until EOF and returns it as a string
// Used to collect CGI output from the child process
std::string CGI::_readFromFd(int fd) const
{
	std::string result;
	char buf[4096];
	ssize_t n;
	//while ((n = read(fd, buf, sizeof(buf))) > 0)
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
		? std::atoi(outHeaders["Status"].c_str()) : 200;
	return body;
}

bool	CGI::_postSupported() const
{
	if (this->_type == HTML || this->_type == CSS || this->_type == JS)
		return false;
	return true;
}

//------------------------------------ EXECUTE CGI ------------------------------------//

// Main function that:
//   1. Sets up environment and pipes
//   2. Forks the process
//   3. Executes the CGI script via execve()
//   4. Reads CGI output
//   5. Builds a proper HTTP response
// Returns: A complete HTTP/1.1 response as a string

std::string CGI::executeCgi(const HttpRequest &request, Server &server, int clientFd, Context &context)
{
	try
	{
		//1. Build environment and create pipes
		std::vector<std::string> cgiEnv = _buildCgiEnv(request, server, request.path);
		std::vector<char*> envp = _envVecToCharPtr(cgiEnv);

		int pipeIn[2];
		int pipeOut[2];
		if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
			throw std::runtime_error("[CGI ERROR] pipe() failed");

		pid_t pid = fork();
		if (pid < 0)
		{
			close(pipeIn[0]);
			close(pipeIn[1]);
			close(pipeOut[0]);
			close(pipeOut[1]);
			throw std::runtime_error("[CGI ERROR] fork() failed");
		}

		//2. Child process
		if (pid == 0)
		{
			server.setFork(1);
			(void)clientFd;

			for (size_t i = 0; i < context.allServerFds.size(); i++)
				close(context.allServerFds[i]);
			for (size_t i = 0; i < context.allClientFds.size(); i++)
				close(context.allClientFds[i]);

			dup2(pipeIn[0], STDIN_FILENO);
			dup2(pipeOut[1], STDOUT_FILENO);
			
			//Close unused ends to prevent deadlock
			close(pipeIn[1]);
			close(pipeOut[0]);
			close(pipeIn[0]);
			close(pipeOut[1]);

			char *argv[3]; 	//Prepare args for execve()
			if (this->_interpreter.empty()) // Direct binary execution (.cgi or already executable script)
			{
				argv[0] = const_cast<char *>(this->_path.c_str());
				argv[1] = NULL;
				execve(this->_path.c_str(), argv, &envp[0]);
			}
			else // Interpreter execution (.py, .sh, etc.)
			{
				argv[0] = const_cast<char *>(this->_interpreter.c_str());
				argv[1] = const_cast<char *>(this->_path.c_str());
				argv[2] = NULL;
				execve(this->_interpreter.c_str(), argv, &envp[0]);
			}
			std::cerr << "execve failed: " << strerror(errno) << std::endl;
			close(pipeIn[0]);
			close(pipeOut[1]);

			return "";
		}
		//3. Parent process
		close(pipeIn[0]); // Close read end of stdin pipe
		close(pipeOut[1]); // Close write end of stdout pipe

		if (request.method == "POST")	// If POST, send body to child's stdin
		{
			if (!this->_postSupported())
				throw std::runtime_error("[CGI ERROR] Method " + request.method + " not allowed for this CGI script");
			std::string body;
			for (std::map<std::string, std::string>::const_iterator it = request.body.begin();
				it != request.body.end(); it++)
				body += it->first + "=" + it->second + "&";
			if (!body.empty())
				body.erase(body.size() - 1);
			write(pipeIn[1], body.c_str(), body.size());
		}
		
		close(pipeIn[1]); // Close write end of pipeIn so the child sees EOF and terminates

		// 4. Read all CGI output until EOF
		std::string rawOutput = _readFromFd(pipeOut[0]);
		close(pipeOut[0]);

		// Wait for CGI child to exit
		int status = 0;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			throw std::runtime_error("[CGI ERROR] process exited with status " + toString(WEXITSTATUS(status)));
		if (WIFSIGNALED(status))
			throw std::runtime_error("[CGI ERROR] process killed by signal " + toString(WTERMSIG(status)));

		//5. Parse and construct HTTP response
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
	catch (const std::exception& e)
	{
		std::ostringstream err;
		err << "HTTP/1.1 500 Internal Server Error\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: "
			<< strlen(e.what()) + 50
			<< "\r\n\r\n"
			<< "<html><body><h1>CGI Error</h1><p>"
			<< e.what() << "</p></body></html>";
		std::cout << GREEN << err.str() << RESET << std::endl; 
		return err.str();
	}
}

std::string CGI::getExtension() const
{
	return this->_extension;
}

std::string  CGI::getPath() const
{
	return this->_path;
}

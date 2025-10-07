#include "Server.hpp"
#include <sys/types.h>
#include <sys/wait.h>

void executeCgi(int client_fd, const HttpRequest &request, const Server &server)
{
	// 1️⃣ Create a pipe for stdin and stdout
	int inputPipe[2];
	int outputPipe[2];
	pipe(inputPipe);
	pipe(outputPipe);

	pid_t pid = fork();
	if (pid == 0)
	{
		// Child process: run the CGI script
		dup2(inputPipe[0], STDIN_FILENO);
		dup2(outputPipe[1], STDOUT_FILENO);
		close(inputPipe[1]);
		close(outputPipe[0]);

		// 2️⃣ Set up environment variables
		std::vector<std::string> env;
		env.push_back("REQUEST_METHOD=" + request.method);
		env.push_back("CONTENT_LENGTH=" + std::to_string(request.body.size()));
		env.push_back("CONTENT_TYPE=" + request.header["Content-Type"]);
		env.push_back("SCRIPT_FILENAME=" + request.path);
		env.push_back("PATH_INFO=" + request.path);
		env.push_back("QUERY_STRING=" + request.query);
		env.push_back("SERVER_PROTOCOL=HTTP/1.1");
		env.push_back("GATEWAY_INTERFACE=CGI/1.1");

		std::vector<char*> envp;
		for (size_t i = 0; i < env.size(); ++i)
			envp.push_back(const_cast<char*>(env[i].c_str()));
		envp.push_back(nullptr);

		// 3️⃣ Prepare args (interpreter + script)
		std::string interpreter = server.getCgiInterpreter(request.path); // e.g. /usr/bin/python3
		char *argv[] = { const_cast<char*>(interpreter.c_str()),
							const_cast<char*>(request.path.c_str()),
							nullptr };

		// 4️⃣ Change working directory
		std::string dir = request.path.substr(0, request.path.find_last_of('/'));
		chdir(dir.c_str());

		// 5️⃣ Execve
		execve(argv[0], argv, envp.data());
		exit(1); // If exec fails
	}

	// Parent process
	close(inputPipe[0]);
	close(outputPipe[1]);

	// 6️⃣ Send request body to child (for POST)
	if (request.method == "POST")
		write(inputPipe[1], request.body.c_str(), request.body.size());
	close(inputPipe[1]);

	// 7️⃣ Read CGI output
	std::string cgiOutput;
	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
		cgiOutput.append(buffer, bytesRead);
	close(outputPipe[0]);

	int status;
	waitpid(pid, &status, 0);

	// 8️⃣ Separate headers/body
	size_t pos = cgiOutput.find("\r\n\r\n");
	std::string headers = cgiOutput.substr(0, pos);
	std::string body = cgiOutput.substr(pos + 4);

	// 9️⃣ Send back to client
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n" << headers << "\r\n\r\n" << body;
	send(client_fd, response.str().c_str(), response.str().size(), MSG_NOSIGNAL);
}


#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

std::vector<char*> buildCgiEnv(const HttpRequest &req, const Server &conf, const std::string &scriptPath)
{
	std::vector<std::string> envVars;

	// --- 1. Static server info ---
	envVars.push_back("SERVER_SOFTWARE=Webserv/1.0");
	envVars.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVars.push_back("SERVER_PROTOCOL=HTTP/1.1");

	// --- 2. Server & connection info ---
	envVars.push_back("SERVER_NAME=" + conf.serverName);
	envVars.push_back("SERVER_PORT=" + std::to_string(conf.port));

	// Client IP
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	getpeername(req.clientFd, (struct sockaddr*)&addr, &addrLen);
	std::string clientIp = inet_ntoa(addr.sin_addr);
	envVars.push_back("REMOTE_ADDR=" + clientIp);

	// --- 3. Request-specific variables ---
	envVars.push_back("REQUEST_METHOD=" + req.method);
	envVars.push_back("SCRIPT_NAME=" + scriptPath);
	envVars.push_back("REQUEST_URI=" + req.url);
	envVars.push_back("QUERY_STRING=" + req.query);

	// If the script has extra path info (e.g., /cgi-bin/script.py/foo/bar)
	size_t scriptEnd = req.url.find(scriptPath);
	if (scriptEnd != std::string::npos && req.url.length() > scriptEnd + scriptPath.length()) {
		std::string pathInfo = req.url.substr(scriptEnd + scriptPath.length());
		envVars.push_back("PATH_INFO=" + pathInfo);
		envVars.push_back("PATH_TRANSLATED=" + conf.root + pathInfo);
	}

	// --- 4. Request headers ---
	std::map<std::string, std::string>::const_iterator it;
	it = req.header.find("Content-Type");
	if (it != req.header.end())
		envVars.push_back("CONTENT_TYPE=" + it->second);

	it = req.header.find("Content-Length");
	if (it != req.header.end())
		envVars.push_back("CONTENT_LENGTH=" + it->second);

	it = req.header.find("Authorization");
	if (it != req.header.end()) {
		envVars.push_back("AUTH_TYPE=Basic");
		envVars.push_back("REMOTE_USER=" + it->second); // optional decoding of Base64 if you want
	}

	// --- 5. Optional & defaults ---
	envVars.push_back("REMOTE_IDENT=");
	envVars.push_back("SERVER_ADMIN=admin@localhost"); // not required but common

	// --- 6. Convert std::vector<std::string> → char*[] for execve ---
	std::vector<char*> envp;
	for (size_t i = 0; i < envVars.size(); ++i)
		envp.push_back(const_cast<char*>(envVars[i].c_str()));
	envp.push_back(NULL);

	return envp;
}

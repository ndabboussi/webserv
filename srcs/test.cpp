#include "Server.hpp"
//This is called when the CGI output pipe reaches EOF — time to build and send the HTTP response:
void Client::finalizeCgiResponse()
{
	int status;
	waitpid(_cgiPid, &status, 0);

	// Parse CGI output
	int cgiStatus = 200;
	std::map<std::string, std::string> headers;
	std::string body;

	size_t headerEnd = _cgiBuffer.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		headerEnd = _cgiBuffer.find("\n\n");

	std::string headersStr = _cgiBuffer.substr(0, headerEnd);
	if (headerEnd != std::string::npos)
		body = _cgiBuffer.substr(headerEnd + 4);

	std::istringstream iss(headersStr);
	std::string line;
	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.size()-1] == '\r')
			line.erase(line.size()-1);
		size_t pos = line.find(':');
		if (pos != std::string::npos)
		{
			std::string key = line.substr(0, pos);
			std::string val = line.substr(pos + 1);
			while (!val.empty() && (val[0] == ' ' || val[0] == '\t'))
				val.erase(0, 1);
			headers[key] = val;
		}
	}

	if (headers.find("Status") != headers.end())
		cgiStatus = std::atoi(headers["Status"].c_str());

	std::ostringstream response;
	response << "HTTP/1.1 " << cgiStatus << " OK\r\n";
	if (headers.find("Content-Length") == headers.end())
		response << "Content-Length: " << body.size() << "\r\n";
	for (auto it = headers.begin(); it != headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n" << body;

	_cgiResponse = response.str();

	send(_clientFd, _cgiResponse.c_str(), _cgiResponse.size(), MSG_NOSIGNAL);

	stopCgi();
	clearCgiBuffer();

	std::cout << GREEN << "[✓] CGI response sent to client" << RESET << std::endl;
}

//Then, after select(), before you handle normal client FDs, check for CGI reads:
for (size_t i = 0; i < clients.size(); ++i)
{
	if (clients[i].isCgiRunning())
	{
		int cgiFd = clients[i].getCgiOutputFd();
		if (FD_ISSET(cgiFd, &readfds))
		{
			char buf[4096];
			ssize_t n = read(cgiFd, buf, sizeof(buf));

			if (n > 0)
				clients[i].appendCgiBuffer(std::string(buf, n));
			else if (n == 0)
			{
				// EOF: CGI finished
				clients[i].finalizeCgiResponse();
			}
			else if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				std::cerr << "[CGI ERROR] read failed on fd " << cgiFd << std::endl;
				clients[i].stopCgi();
			}
		}
	}
}
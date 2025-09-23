#include "parsingRequest.hpp"

int parsePath(HttpRequest &req, const Server &server)
{
	std::vector<Location> tmp = server.getLocations();
	std::vector<Location>::iterator it;
	for (it = tmp.begin(); it != tmp.end(); it++)
	{
		if (it->getPath() == req.path)
		{
			std::map<std::string, std::string> data = it->getData();
			if (data.find("alias") != data.end())
				req.path = data.find("alias")->second;
			else if (server.getData().find("root") != server.getData().end())
				req.path = server.getData().find("root")->second + req.path;
			break ;
		}
	}
	if (req.path[0] == '/')
		req.path.erase(req.path.begin());
	std::cout << "path: " << req.path << std::endl;
	struct stat st;
	if (stat(req.path.c_str(), &st) == 0)
	{
		if (!S_ISREG(st.st_mode))
		{
			if (it != tmp.end())
			{
				if (it->getData().find("index") != it->getData().end())\
					req.path += '/' + it->getData().find("index")->second;
				else
				{
					std::cerr << RED "Error 404: Not found: " << req.path << RESET << std::endl;
					req.error = 404;
					return 1;
				}

			}
		}
	}
	else
	{
		std::cerr << RED "Error 404: Not found: " << req.path << RESET << std::endl;
		req.error = 404;
		return 1;
	}
	if (access(req.path.c_str(), F_OK) != 0)
	{
		std::cerr << RED "Error 404: Not found: " << req.path << RESET << std::endl;
		req.error = 404;
		return 1;
	}
	if (access(req.path.c_str(), R_OK) != 0)
	{
		std::cerr << RED "Error 403: Forbidden" << RESET << std::endl;
		req.error = 403;
		return 1;
	}
	return (0);
}

HttpRequest parseHttpRequest(const std::string &rawRequest, const Server &server)
{
	HttpRequest req;

	std::istringstream requestStream(rawRequest);
	requestStream >> req.method >> req.path >> req.version;
	req.error = 0;
	std::cout << req.method << ", " << req.path << ", " << req.version << std::endl;
	if (req.method.empty() || req.path.empty() || req.version.empty())
	{
		std::cerr << RED "Error 400: Malformed HTTP request received" << RESET << std::endl;//400 bad request
		req.error = 400;
	}
	else
	{
		if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
		{
			std::cerr << RED "Error 405: Method not allowed" << RESET << std::endl;
			req.error = 405;
		}
		else if (req.version != "HTTP/1.1")
		{
			std::cerr << RED "Error 505: HTTP Version Not Supported" << RESET << std::endl;
			req.error = 505;
		}
		else if (parsePath(req, server))
			;
		else
		{
			std::cout << YELLOW "[>] Parsed Request: "
						<< req.method << " " << req.path << " " << req.version
						<< RESET << std::endl;
		}
	}

	return req;
}
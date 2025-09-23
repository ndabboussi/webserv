#include "parsingRequest.hpp"

int findLocations(std::string str, Location &location)
{
	std::vector<Location>::iterator it;
	std::vector<Location> tmp = location.getLocations();

	for (it = tmp.begin(); it != tmp.end(); it++)
	{
		std::map<std::string, std::string> map = it->getData();
		if (str == it->getPath())
		{
			location = *it;
			return 1;
		}
	}
	return 0;
}

int parsePath(HttpRequest &req, const Server &server)
{
	std::vector<Location> tmp = server.getLocations();
	std::map<std::string,std::string> data;
	std::cout << req.path << std::endl;
	std::string newPath = (server.getData().find("root") != server.getData().end())
								? server.getData().find("root")->second : "";
	Location loc = server;
	size_t i = 0;
	size_t end = 0;
	std::string str;

	while (end != std::string::npos)
	{
		data = loc.getData() ;
		if (data.find("alias") != data.end())
			newPath = data.find("alias")->second;
		i = req.path.find('/', i);
		end = req.path.find('/', i + 1);
		if (end != std::string::npos)
			str = req.path.substr(i, end - i);
		else
			str = req.path.substr(i, req.path.size());
		i++;
		findLocations(str, loc);
	}
	if (str[0] == '/' && str.size() == 1)
		str = "";
	data = loc.getData() ;
	if (data.find("alias") != data.end())
		newPath = data.find("alias")->second + str;
	else
		newPath += str;
	req.path = newPath;
	if (req.path[0] == '/')
		req.path.erase(req.path.begin());
	struct stat st;
	if (stat(req.path.c_str(), &st) == 0)
	{
		if (!S_ISREG(st.st_mode))
		{
			if (loc.getData().find("index") != loc.getData().end())
				req.path += '/' + loc.getData().find("index")->second;
			else
			{
				std::cerr << RED "inside stat Error 404: Not found: " << req.path << RESET << std::endl;
				req.error = 404;
				return 1;
			}
		}
	}
	else
	{
		std::cerr << RED "in stat, Error 404: Not found: " << req.path << RESET << std::endl;
		req.error = 404;
		return 1;
	}
	if (access(req.path.c_str(), F_OK) != 0)
	{
		std::cerr << RED "in access, Error 404: Not found: " << req.path << RESET << std::endl;
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
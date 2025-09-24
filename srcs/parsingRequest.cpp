#include "parsingRequest.hpp"

static int error404(HttpRequest &req, std::string &tmp)
{
	std::cerr << RED "Error 404: Not found: " << tmp << RESET << std::endl;
	req.error = 404;
	return 1;
}

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

static int isAFile(std::string path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0)
	{
		if (!S_ISREG(st.st_mode))
			return 0;
	}
	else
		return -1;
	return 1;
}

static int buildPath(std::string &newPath, std::string oldPath, Location &loc, HttpRequest &req)
{
	size_t i = 0;
	size_t end = 0;
	std::map<std::string,std::string> data;
	std::string str;

	while (end != std::string::npos)
	{
		newPath += str;
		data = loc.getData();
		if (data.find("alias") != data.end())
			newPath = data.find("alias")->second;
		i = oldPath.find('/', i);
		end = oldPath.find('/', i + 1);
		if (end != std::string::npos)
			str = oldPath.substr(i, end - i);
		else
			str = oldPath.substr(i, oldPath.size());
		i++;
		findLocations(str, loc);
	}
	if (str[0] == '/' && str.size() == 1)
		str.clear();
	data = loc.getData();
	if (data.find("alias") != data.end())
	{
		newPath = data.find("alias")->second;
		if (newPath[0] == '/')
			newPath.erase(newPath.begin());
		std::string tmp = newPath + str;
		if (isAFile(tmp) < 0)
			return error404(req, tmp);
		newPath += str;
	}
	else
		newPath += str;
	return 0;
}

static int checkAccess(HttpRequest &req)
{
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
	return 0;
}

int parsePath(HttpRequest &req, const Server &server)
{
	std::vector<Location> tmp = server.getLocations();
	std::string newPath = (server.getData().find("root") != server.getData().end())
								? server.getData().find("root")->second : "";
	Location loc = server;
	std::map<std::string,std::string> data;
	std::string str;
	
	if (buildPath(newPath, req.path, loc, req)) //build the first part of the path (composed by directories)
		return 1;
	req.path = newPath;
	if (req.path[0] == '/')
		req.path.erase(req.path.begin());
	int res = isAFile(req.path);
	if (res == 0 && req.method == "GET")//if the path is a directory
	{
		data = loc.getData();
		std::map<std::string, std::string>::const_iterator it = data.find("index");
		if (it != data.end())
			req.path += "/" + it->second;
		else
			return error404(req, req.path);
	}
	else if (res < 0) //if the path isn't found
		return error404(req, req.path);
	if (checkAccess(req))
		return 1;
	return (0);
}

static int parseHeader(HttpRequest req, std::istringstream &requestStream)
{
	std::string str;
	while (std::getline(requestStream, str))
	{
		if (str == "\r")
			break;
		str.erase(str.find_last_of('\r'));
		std::string key, value;
		size_t pos = str.find(':');
		key = str.substr(0, pos);
		value = str.substr(pos + 1, str.size() - pos);
		std::cout << "Key: " << key << ", value: " << value << std::endl;
		req.header.insert(std::make_pair(key, value));
	}
	if (str != "\r")
		return 1;
	return 0;
}

static int parseBody(HttpRequest req, std::istringstream &requestStream)
{
	std::string str;
	std::string res = "";
	while (std::getline(requestStream, str))
	{
		// if (str == "\r")
		// 	break;
		//str.erase(str.find_last_of('\r'));
		res += str + "\n";// est ce que je dois recup comme c'est recu ? avec les \r\n ?		
	}
	if (str != "\r")
		return 1;
	req.body = res;
	return 0;
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
		return req;
	}
	if (parseHeader(req, requestStream))
		;//put error here
	if (req.method == "POST" && parseBody(req, requestStream))
		;//put error here
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
	return req;
}
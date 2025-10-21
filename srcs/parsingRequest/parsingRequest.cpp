#include "parsingRequest.hpp"

#define MAX_URI_LENGTH 8192


static int parseHeader(HttpRequest &req, std::istringstream &requestStream, const Server &server)
{
	std::string str;
	int i = 0;
	std::getline(requestStream, str);
	while (std::getline(requestStream, str))
	{
		if (str == "\r")
			break;
		if (str.find_last_of('\r') != std::string::npos)
			str.erase(str.find_last_of('\r'));
		std::string key, value;
		size_t pos = str.find(':');
		if (pos == std::string::npos)
			return error400(req);
		key = str.substr(0, pos);
		if (pos + 1 >= str.size() || pos + 2 >= str.size())
			return error400(req);
		value = str.substr(pos + 2, str.size() - pos + 2);
		req.header.insert(std::make_pair(key, value));
		i++;
	}
	if (str != "\r")
		return 1;
	
	if (req.header.find("Content-Length") != req.header.end() && std::atoll(req.header.find("Content-Length")->second.c_str()) > server.getMaxBodyClientSize())
		return error413(req);
	return 0;
}

static int recupBodySize(const std::string &rawRequest, HttpRequest &req)
{
	if (req.header.find("Content-Length") == req.header.end() && req.header.find("Transfer-Encoding") != req.header.end())
	{
		size_t pos = rawRequest.find("\r\n\r\n") + 4;
		if (pos == std::string::npos)
			return error400(req);
		std::ostringstream oss;
		oss << rawRequest.size() - pos;
		req.header.insert(std::make_pair("Content-Length", oss.str()));
	}
	return 0;
}

static int checkErrors(HttpRequest &req)
{
	if (req.method.empty() || req.path.empty() || req.version.empty())
		return error400(req);
	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
		return error501(req);
	else if (req.version != "HTTP/1.1")
		return error505(req);
	return 0;
}

static int checkMethod(HttpRequest &req)
{
	if ((!(req.methodPath & 2) && req.method == "POST") || (!(req.methodPath & 1) && req.method == "GET")
			|| (!(req.methodPath & 4) && req.method == "DELETE"))
		return error405(req);
	return 0;
}

static int restoreLocations(const Location &location, std::string root)
{
	std::vector<Location>				loc = location.getLocations();
	std::map<std::string, std::string>	map;
	std::string							str;
	
	if (root.size() > 0 && root[0] == '/')
		root.erase(root.begin());
	for (size_t i = 0; i < loc.size(); i++)
	{
		map = loc[i].getData();
		if (map.find("alias") != map.end())
		{
			str = map.find("alias")->second;
			if (str.size() > 0 && str[0] == '/')
				str.erase(str.begin());
		}
		else
			str = root + loc[i].getPath();
		if (isAFile(str) < 0)
			mkdir(str.c_str(), 0755);
		restoreLocations(loc[i], str);
	}
	return 0;
}

static int removeDir(const char *path)
{
	DIR* dir = opendir(path);
    if (!dir)
		return 1;

    dirent* entry;
	while ((entry = readdir(dir)))
	{
		if (!strcmp(entry->d_name , ".") || !strcmp(entry->d_name, ".."))
			continue ;
		std::string newPath = std::string(path) + '/' + entry->d_name;
		struct stat st;
		if (lstat(newPath.c_str(), &st) == 0)
		{
			if (S_ISDIR(st.st_mode) && removeDir(newPath.c_str()))
				return closedir(dir), 1;
			else if (!S_ISDIR(st.st_mode) && std::remove(newPath.c_str()) < 0)
				return closedir(dir), 1;
		}
	}
	closedir(dir);
	if (rmdir(path) < 0)
		return 1;
	return 0;
}

int checkDot(std::string &path, HttpRequest &req)
{
	std::string dir;
	int i = 0;

	for (size_t j = 0; j < path.size(); j++)
	{
		if (path[j] && path[j] != '/' && i == 0)
		{
			i = 1;
			dir += path[j];
		}
		else if (path[j] == '/')
		{
			if (dir == "..")
				return error400(req);
			dir.clear();
			i = 0;
		}
		else
			dir += path[j];
	}
	if (dir == "..")
        return error400(req);
	return 0;
}

static int deleteRessource(HttpRequest &req, const Server &server)
{
	int			res = isAFile(req.path);
	std::string	root;
	std::vector<char> tmp(500);
	std::map<std::string, std::string>	data = server.getData();

	if (checkDot(req.path, req))
		return 1;
	if (data.find("root") != data.end())
		root = data.find("root")->second;
	else
	{
		if (!getcwd(tmp.data(), 500))
			return error500(req);
		root = tmp.data();
	}
	if (res > 0)
	{
		if (std::remove(req.path.c_str()) < 0)
			return error500(req);
	}
	else if (res == 0 && removeDir(req.path.c_str()))
		return error500(req);
	if (res == 0)
		restoreLocations(server, root);
	return 0;
}

HttpRequest parseHttpRequest(const std::string &rawRequest, Server &server)
{
	HttpRequest req;

	std::istringstream requestStream(rawRequest, std::ios::binary);
	requestStream >> req.method >> req.path >> req.version;
	req.statusCode = 0;
	if (checkErrors(req))// check if the request is supported or this is the right http version
		return req;
	if (req.path.size() > MAX_URI_LENGTH)
	return  error414(req), req;
	req.url = req.path;
	if (parseHeader(req, requestStream, server))//parsing header
		return req;
	if (recupBodySize(rawRequest, req))// if the request was chunked, recup the size of the body ans store it
		return req;
	if (!parsePath(req, server))
	{
		if (checkMethod(req)) //If the method can't be used in the directory
			return req;
		if (!req.isCgi && req.method == "POST" && parseBody(req, requestStream))//If request is Post -> parse the body of the request and
			return req;											  //if there is an error, return the error
		if (req.method == "DELETE" && deleteRessource(req, server))//If one of the ressourses couldn't be removed raise error
			return req;
		std::cout << YELLOW "[>] Parsed Request: "
					<< req.method << " " << req.path << " " << req.version
					<< RESET << std::endl;
	}
	return req;
}

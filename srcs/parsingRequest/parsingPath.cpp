#include "Client.hpp"

static int findLocations(std::string str, Location &location)
{
	std::vector<Location>::iterator it;
	std::vector<Location> tmp = location.getLocations();

	for (it = tmp.begin(); it != tmp.end(); it++)
	{
		if (str == it->getPath())
		{
			location = *it;
			return 1;
		}
	}
	return 0;
}

int isAFile(std::string path)
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
	size_t								i = 0;
	size_t								end = 0;
	std::map<std::string,std::string>	data = loc.getData();
	Location							prev;
	std::string							serverRoot = (data.find("root") != data.end())
											? data.find("root")->second : "";
	std::string							str;

	if (newPath == "/")
		newPath.clear();
	oldPath = (oldPath[0] != '/') ? '/' + oldPath : oldPath;
	while (end != std::string::npos)
	{
		newPath += str;
		data = loc.getData();
		if (!loc.getRedirect().path.empty())
		{
			req.statusCode = loc.getRedirect().redirCode;
			req.url = loc.getRedirect().path;
			return 1;
		}
		if (data.find("alias") != data.end() && prev.getPath() != loc.getPath())
			newPath = data.find("alias")->second;
		else if (data.find("root") != data.end() && data.find("root")->second != serverRoot
				&& prev.getPath() != loc.getPath())
			newPath = data.find("root")->second + loc.getPath();
		i = oldPath.find('/', i);
		end = oldPath.find('/', i + 1);
		if (end != std::string::npos)
			str = oldPath.substr(i, end - i);
		else
			str = oldPath.substr(i, oldPath.size());
		i++;
		prev = loc;
		findLocations(str, loc);
	}
	data = loc.getData();
	if (!loc.getRedirect().path.empty())
	{
		req.statusCode = loc.getRedirect().redirCode;
		req.url = loc.getRedirect().path;
		return 1;
	}
	if (data.find("alias") != data.end() && prev.getPath() != loc.getPath())
	{
		if (req.url[req.url.size() - 1] == '/')
			newPath = data.find("alias")->second + '/';
		else
			newPath = data.find("alias")->second;
		if (newPath[0] == '/')
			newPath.erase(newPath.begin());
		if (isAFile(newPath + str) >= 0)
			newPath += str;
	}
	else if (data.find("root") != data.end() && data.find("root")->second != serverRoot
			&& prev.getPath() != loc.getPath())
	{
		if (req.url[req.url.size() - 1] == '/')
			newPath = data.find("root")->second + loc.getPath() + '/';
		else
			newPath = data.find("root")->second + loc.getPath();
		if (newPath[0] == '/')
			newPath.erase(newPath.begin());
		if (isAFile(newPath + str) >= 0)
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
		std::cerr << RED "in access, Error 404: Not found " << req.path << RESET << std::endl;
		req.statusCode = 404;
		return 1;
	}
	if (access(req.path.c_str(), R_OK) != 0)
		return error403(req, req.path);
	return 0;
}

static int fillIndexFile(HttpRequest &req)
{
    DIR *dir = opendir(req.path.c_str());
    if (dir == NULL)
		return error500(req);
	req.autoIndexFile =
        "<!doctype html>\n<html lang=\"en\">\n"
        "\t<head>\n"
        "\t\t<meta charset=\"utf-8\" />\n"
        "\t\t<title>Index of " + req.url + "</title>\n"
		"\t\t<link rel=\"stylesheet\" href=\"/siteUtils/styles.css\">\n"
        "\t\t<link rel=\"stylesheet\" href=\"/siteUtils/sidebar.css\">\n"
        "\t\t<style>\n"
        "\t\t\tbody {font-family: 'Segoe UI', Arial, sans-serif;"
        "display: flex; flex-direction: column; justify-content: center; align-items: center; margin: 0;}\n"
        "\t\t\th1 { margin-bottom: 0.5em; }\n"
        "\t\t\tul { list-style-type: none; padding: 0; }\n"
        "\t\t\tli { margin: 0.3em 0; }\n"
        "\t\t\tli a { text-decoration: none; color: #007acc; }\n"
        "\t\t\tli a:hover { text-decoration: underline; }\n"
        "\t\t</style>\n"
        "\t</head>\n"
        "\t<body>\n"
        "\t\t<div>\n"
        "\t\t\t<h1>Index of " + req.url + " directory </h1>\n"
        "\t\t\t<ul>\n";

    struct dirent *entry;

	if (req.header.find("Host") == req.header.end())
		return error400(req);
	
	std::string port = req.header.find("Host")->second;
	if (req.url[req.url.size() - 1] == '/')
		req.url.erase(req.url.size() - 1, 1);
    while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_name[0] != '.')
			req.autoIndexFile += "\t\t\t\t<li><a href=\"http://" + port + req.url + '/' + std::string(entry->d_name)
				+ "\">" + std::string(entry->d_name) + "</a></li>\n";
	}
	req.autoIndexFile += "\t\t\t</ul>\n"
        "\t\t\t<div style=\"margin-top:2em;\">\n"
        "\t\t\t\t<a class=\"button\" href=\"/\">🏠 Go back to Home Page</a>\n"
        "\t\t\t</div>\n"
        "\t\t</div>\n"
        "\t\t<div id=\"sidebar-container\"></div>\n"
        "\t\t<script src=\"/siteUtils/cookies.js\"></script>\n"
        "\t</body>\n"
        "</html>";
    closedir(dir);
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
	req.methodPath = loc.getMethods();

	if (req.url == "/register" || req.url == "/login" || req.url == "/logout" || req.url == "/me")
			return (0);
  
	int res = isAFile(req.path);
	if (res == 0 && req.method == "GET")//if the path is a directory
	{
		if (req.path[req.path.size() - 1] != '/')
		{
			if (req.url[req.url.size() - 1] == '/')
				req.url.erase(req.url.end() - 1);
			return error301(req);
		}
		data = loc.getData();
		std::map<std::string, std::string>::const_iterator it = data.find("index");
		if (it != data.end())
			req.path +=  it->second;
		else if (loc.getAutoIndex())
		{
			if (fillIndexFile(req))
				return 1;
		}
		else if (!loc.getAutoIndex())
			return error403(req, req.path);
	}
	else if (res < 0) //if the path isn't found
		return error404(req, req.path);
	else if (res == 1)
	{
		req.isCgi = false;
		std::vector<std::string> cgiExt = loc.getCgiExt();
		for (size_t i = 0; i < cgiExt.size(); i++)
		{
			if (req.path.size() >= cgiExt[i].size() && req.path.compare(req.path.size() - cgiExt[i].size(),
								cgiExt[i].size(),
								cgiExt[i]) == 0)
			{
				req.isCgi = true;
				break;
			}
		}
		
		if (!req.isCgi && req.path.find("cgi-bin") != std::string::npos)
			req.isCgi = true;

	}

	if (checkAccess(req))
		return 1;

	return (0);
}

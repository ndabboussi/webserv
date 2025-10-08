#include "parsingRequest.hpp"

static int error404(HttpRequest &req, std::string &tmp)
{
	std::cerr << RED "Error 404: Not found: " << tmp << RESET << std::endl;
	req.statusCode = 404;
	return 1;
}

static int error301(HttpRequest &req)
{
	std::cerr << RED "Error 301: Moved permanently" << RESET << std::endl;
	req.statusCode = 301;
	return 1;
}

static int error403(HttpRequest &req, std::string &tmp)
{
	std::cerr << RED "Error 403: Forbidden: " << tmp << RESET << std::endl;
	req.statusCode = 403;
	return 1;
}

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
		std::cerr << RED "in access, Error 404: Not found " << req.path << RESET << std::endl;
		req.statusCode = 404;
		return 1;
	}
	if (access(req.path.c_str(), R_OK) != 0)
	{
		std::cerr << RED "Error 403: Forbidden" << RESET << std::endl;
		req.statusCode = 403;
		return 1;
	}
	return 0;
}

static int fillIndexFile(HttpRequest &req)
{
    DIR *dir = opendir(req.path.c_str());
    if (dir == NULL)
	{
		std::cerr << RED "Error 500: Internal server error: "<< RESET << std::endl;
		req.statusCode = 500;
        return 1;
    }
	req.autoIndexFile = "<!doctype html>\n<html lang=\"en\">\n\t<head>\n\t\t<meta charset=\"utf-8\" />\n\t\t<title>Page List</title>\n";
	req.autoIndexFile += "\t\t<link rel=\"stylesheet\"  href=\"/siteUtils/sidebar.css\">\n";
	req.autoIndexFile += "\t\t<style>\n\t\t\tbody{font-family: \"Segoe UI\", Arial, sans-serif;\n\t\t\tdisplay: flex;\n";
	req.autoIndexFile += "\t\t\tflex-direction: column;\n\t\t\tjustify-content: center;\n\t\t\talign-items: center;\n\t\t\tmargin: 0;\n";
	req.autoIndexFile += "\t\t}\n\t\t</style>\n";
	req.autoIndexFile += "\t</head>\n\t<body>\n\t\t<div>\n\t\t\t<h1>List of files in the directory</h1>\n";
	req.autoIndexFile += "\t\t\t<ul>\n";

    struct dirent *entry;
	if (req.header.find("Host") == req.header.end())
	{
		std::cerr << RED "Error 400: Bad request: "<< RESET << std::endl;
		req.statusCode = 400;
        return 1;
	}
	std::string port = req.header.find("Host")->second;
	if (req.url[req.url.size() - 1] == '/')
		req.url.erase(req.url.size() - 1, 1);
    while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_name[0] != '.')
			req.autoIndexFile += "\t\t\t\t<li><a href=\"http://" + port + req.url + '/' + std::string(entry->d_name)
				+ "\">" + std::string(entry->d_name) + "</a></li>\n";
	}
	req.autoIndexFile += "\t\t\t</ul>\n\t\t</div>\n";
	req.autoIndexFile += "\t\t<div id=\"sidebar-container\"></div>\n\t\t<script src=\"/siteUtils/cookies.js\"></script>\n\t</body>\n</html>";
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
	int res = isAFile(req.path);
	if (res == 0 && req.method == "GET")//if the path is a directory
	{
		if (req.path[req.path.size() - 1] != '/')
			return error301(req);
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
	if (checkAccess(req))
		return 1;
	req.methodPath = loc.getMethods();

	// CGI config file ?

	std::vector<std::string> cgiExt = loc.getCgiExt();
	if (cgiExt.empty())
		req.isCgi = false;
	else
		req.isCgi = true;
	
	std::cout << "HELOOOOOOOO" << loc.getPath() << std::endl;
	return (0);
}

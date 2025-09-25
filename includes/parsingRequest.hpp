#ifndef PARSINGREQUEST_HPP

# define PARSINGREQUEST_HPP

# include <sys/stat.h>
# include "Server.hpp"

class	 Server;

struct HttpRequest
{
	int			error;
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> header;
	std::map<std::string, std::string> body;
	std::string JsonBody;
};

HttpRequest	parseHttpRequest(const std::string &rawRequest, const Server &server);
int			parsePath(HttpRequest &req, const Server &server);
int			parseBody(HttpRequest &req, std::istringstream &requestStream);

# endif
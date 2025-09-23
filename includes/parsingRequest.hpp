#ifndef PARSINGREQUEST_HPP

# define PARSINGREQUEST_HPP

# include <sys/stat.h>
# include "Server.hpp"

struct HttpRequest
{
	int			error;
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> header;
};

HttpRequest	parseHttpRequest(const std::string &rawRequest, const Server &server);

# endif
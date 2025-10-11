#ifndef PARSINGREQUEST_HPP

# define PARSINGREQUEST_HPP

# include <sys/stat.h>
# include "Server.hpp"

class	 Server;

struct HttpRequest
{
	int									statusCode;
	std::string 						method;
	std::string							path;
	uint8_t								methodPath;
	std::string							version;
	std::map<std::string, std::string>	header;
	std::map<std::string, std::string>	body;
	std::vector<std::string>			fileNames;
	std::string							url;
	std::string							autoIndexFile;
	bool								isCgi;
	int									serverPort;
};

HttpRequest	parseHttpRequest(const std::string &rawRequest, Server &server);
int			parsePath(HttpRequest &req, const Server &server);
int			parseBody(HttpRequest &req, std::istringstream &requestStream);
int			isAFile(std::string path);
int			error301(HttpRequest &req);
int			error400(HttpRequest &req);
int			error403(HttpRequest &req, std::string &tmp);
int			error404(HttpRequest &req, std::string &tmp);
int			error405(HttpRequest &req);
int			error413(HttpRequest &req);
int			error500(HttpRequest &req);
int			error501(HttpRequest &req);
int			error505(HttpRequest &req);

# endif
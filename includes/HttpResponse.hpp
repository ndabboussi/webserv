#ifndef HTTPRESPONSE_HPP

# define HTTPRESPONSE_HPP

# include <sys/stat.h>
# include "Server.hpp"

class	 Server;

struct HttpResponse
{
	int									code;
	std::string							statusLine;
	std::map<std::string, std::string>	headers;
	std::map<std::string, std::string>	body;
	std::string							redirectTo;
	bool								autoIndex;
	std::string							autoIndexFile;
};

# endif
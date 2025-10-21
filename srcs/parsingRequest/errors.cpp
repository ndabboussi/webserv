#include "parsingRequest.hpp"

int error301(HttpRequest &req)
{
	std::cerr << RED "Error 301: Moved permanently" << RESET << std::endl;
	req.statusCode = 301;
	return 1;
}

int error400(HttpRequest &req)
{
	std::cerr << RED "Error 400: Bad request "<< RESET << std::endl;
	req.statusCode = 400;
	return 1;
}

int error403(HttpRequest &req, std::string &tmp)
{
	std::cerr << RED "Error 403: Forbidden: " << tmp << RESET << std::endl;
	req.statusCode = 403;
	return 1;
}

int error404(HttpRequest &req, std::string &tmp)
{
	std::cerr << RED "Error 404: Not found: " << tmp << RESET << std::endl;
	req.statusCode = 404;
	return 1;
}

int error405(HttpRequest &req)
{
	std::cerr << RED "Error 405: Method not allowed" << RESET << std::endl;
	req.statusCode = 405;
	return 1;
}

int error413(HttpRequest &req)
{
	std::cerr << RED "Error 413: Entity Too Large" << RESET << std::endl;
	req.statusCode = 413;
	return 1;
}

int error500(HttpRequest &req)
{
	std::cerr << RED "Error 500: Internal server error"<< RESET << std::endl;
	req.statusCode = 500;
	return 1;
}

int error501(HttpRequest &req)
{
	std::cerr << RED "Error 501: Not inplemented: "<< RESET << std::endl;
	req.statusCode = 501;
	return 1;
}

int error505(HttpRequest &req)
{
	std::cerr << RED "Error 505: HTTP Version Not Supported" << RESET << std::endl;
	req.statusCode = 505;
	return 1;
}

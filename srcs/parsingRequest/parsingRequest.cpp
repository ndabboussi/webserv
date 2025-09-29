#include "parsingRequest.hpp"

static int parseHeader(HttpRequest &req, std::istringstream &requestStream)
{
	std::string str;
	int i = 0;
	std::getline(requestStream, str);
	while (std::getline(requestStream, str))
	{
		if (str == "\r" && i != 0)
			break;
		str.erase(str.find_last_of('\r'));
		std::string key, value;
		size_t pos = str.find(':');
		if (pos == std::string::npos)
		{
			std::cerr << RED "Error 400: Bad request" << RESET << std::endl;//400 bad request
			req.error = 400;
			return 1;
		}
		key = str.substr(0, pos);
		if (pos + 1 >= str.size() || pos + 2 >= str.size())
		{
			std::cerr << RED "Error 400: Bad request" << RESET << std::endl;//400 bad request
			req.error = 400;
			return 1;
		}
		value = str.substr(pos + 2, str.size() - pos + 2);
		req.header.insert(std::make_pair(key, value));
		i++;
	}
	if (str != "\r")
		return 1;
	return 0;
}


HttpRequest parseHttpRequest(const std::string &rawRequest, const Server &server)
{
	HttpRequest req;

	std::istringstream requestStream(rawRequest, std::ios::binary);
	requestStream >> req.method >> req.path >> req.version;
	req.error = 0;
	req.url = req.path;
	if (req.method.empty() || req.path.empty() || req.version.empty())
	{
		std::cerr << RED "Error 400: Malformed HTTP request received" << RESET << std::endl;//400 bad request
		req.error = 400;
		return req;
	}
	if (parseHeader(req, requestStream))
		return req;
	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
	{
		std::cerr << RED "Error 400: Bad request" << RESET << std::endl;
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
		if ((!(req.methodPath & 2) && req.method == "POST") || (!(req.methodPath & 1) && req.method == "GET")
			|| (!(req.methodPath & 4) && req.method == "DELETE"))
		{
			std::cerr << RED "Error 405: Method not allowed" << RESET << std::endl;
			req.error = 405;
			return req;
		}
		if (req.method == "POST" && parseBody(req, requestStream))
			return req;
		std::cout << YELLOW "[>] Parsed Request: "
					<< req.method << " " << req.path << " " << req.version
					<< RESET << std::endl;
	}
	return req;
}
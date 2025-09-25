#include "parsingRequest.hpp"

static int parseHeader(HttpRequest &req, std::istringstream &requestStream)
{
	std::string str;
	int i = 0;
	while (std::getline(requestStream, str))
	{
		if (str == "\r" && i != 0)
			break;
		str.erase(str.find_last_of('\r'));
		std::string key, value;
		size_t pos = str.find(':');
		key = str.substr(0, pos);
		value = str.substr(pos + 1, str.size() - pos);
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
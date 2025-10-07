#include "parsingRequest.hpp"

static int parseHeader(HttpRequest &req, std::istringstream &requestStream, const Server &server)
{
	std::string str;
	int i = 0;
	std::getline(requestStream, str);
	while (std::getline(requestStream, str))
	{
		if (str == "\r")
			break;
		str.erase(str.find_last_of('\r'));
		std::string key, value;
		size_t pos = str.find(':');
		if (pos == std::string::npos)
		{
			std::cerr << RED "Error 400: Bad request" << RESET << std::endl;//400 bad request
			req.statusCode = 400;
			return 1;
		}
		key = str.substr(0, pos);
		if (pos + 1 >= str.size() || pos + 2 >= str.size())
		{
			std::cerr << RED "Error 400: Bad request" << RESET << std::endl;//400 bad request
			req.statusCode = 400;
			return 1;
		}
		value = str.substr(pos + 2, str.size() - pos + 2);
		req.header.insert(std::make_pair(key, value));
		i++;
	}
	if (str != "\r")
		return 1;
	
	if (req.header.find("Content-Length") != req.header.end() && std::atoll(req.header.find("Content-Length")->second.c_str()) > server.getMaxBodyClientSize())
	{
		std::cerr << RED "Error 413: Entity Too Large" << RESET << std::endl;
		req.statusCode = 413;
		return 1;
	}
	return 0;
}


HttpRequest parseHttpRequest(const std::string &rawRequest, const Server &server)
{
	HttpRequest req;

	std::istringstream requestStream(rawRequest, std::ios::binary);
	requestStream >> req.method >> req.path >> req.version;
	req.statusCode = 0;
	req.url = req.path;
	if (req.method.empty() || req.path.empty() || req.version.empty())//If either on of these fields is empty, this is a bad request
	{
		std::cerr << RED "Error 400: Bad request" << RESET << std::endl;
		req.statusCode = 400;
		return req;
	}
	if (parseHeader(req, requestStream, server))
		return req;
	if (req.header.find("Content-Length") == req.header.end() && req.header.find("Transfer-Encoding") != req.header.end())
	{
		size_t pos = rawRequest.find("\r\n\r\n") + 4;
		if (pos == std::string::npos)
		{
			std::cerr << RED "Error 400: Bad request" << RESET << std::endl;
			req.statusCode = 400;
			return req;
		}
		std::ostringstream oss;
    	oss << rawRequest.size() - pos;
		req.header.insert(std::make_pair("Content-Length", oss.str()));
	}
	if (req.method != "GET" && req.method != "POST" && req.method != "DELETE")
	{
		std::cerr << RED "Error 400: Bad request" << RESET << std::endl;
		req.statusCode = 405;
	}
	else if (req.version != "HTTP/1.1")
	{
		std::cerr << RED "Error 505: HTTP Version Not Supported" << RESET << std::endl;
		req.statusCode = 505;
	}
	else if (parsePath(req, server))
		;
	else
	{
		if ((!(req.methodPath & 2) && req.method == "POST") || (!(req.methodPath & 1) && req.method == "GET")
			|| (!(req.methodPath & 4) && req.method == "DELETE"))//If the method can't be used in the directory
		{
			std::cerr << RED "Error 405: Method not allowed" << RESET << std::endl;
			req.statusCode = 405;
			return req;
		}
		if (req.method == "POST" && parseBody(req, requestStream))//If request is Post -> parse the body of the request and
			return req;											  //if there is an error, return the error
		std::cout << YELLOW "[>] Parsed Request: "
					<< req.method << " " << req.path << " " << req.version
					<< RESET << std::endl;
	}
	return req;
}
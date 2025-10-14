#include "Response.hpp"
#include "Server.hpp"
#include "CGI.hpp"

//Operators----------------------------------------------------------------

Response &Response::operator=(Response const &src)
{
	if (this == &src)
		return (*this);
	return (*this);
}

//Constructor/Destructors--------------------------------------------------

Response::Response(int clientFd, HttpRequest const &request, Server &server): _clientFd(clientFd), _request(request), _server(server)
{}

Response::Response(Response const &src) : _clientFd(src._clientFd), _server(src._server), _request(src._request)
{
	*this = src;
	return ;
}

Response::~Response(void)
{}

//Member functions------------------------------------------------------

//------------------------- HEADERS UTILS -------------------------//

std::string	toString(size_t value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

void	Response::setStatusCode()
{
	if (this->_request.statusCode >= 400)
		return;
	if (this->_request.method == "DELETE")
	{
		this->_code = 204; //specifying that we won't send body
		return;
	}
	else if (this->_request.statusCode == 0)
	{
		this->_code = 200;
		return;
	}
	else
		this->_code = this->_request.statusCode;
}

void	Response::setStatusLine()
{
	std::ostringstream line;
	line << "HTTP/1.1 " << this->_code << " " << statusCodeResponse(this->_code) << "\r\n";
	this->_statusLine = line.str();
}

//Format current date for HTTP header
std::string setDate()
{
	char buffer[1000];
	time_t now = time(0);
	struct tm *tm = gmtime(&now);
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm);
	std::string date = buffer;
	date.append("\r\n");
	return date;
}

std::string setConnection(HttpRequest const &request)
{
	// Look for "Connection" in the request headers
	std::map<std::string, std::string>::const_iterator it = request.header.find("Connection");
	if (it != request.header.end())
	{
		std::string value = it->second;
		if (value == "keep-alive")
			return "keep-alive\r\n";
		if (value == "close")
			return "close\r\n";
	}
	// Default for HTTP/1.1 is keep-alive
	return "keep-alive\r\n";
}

void Response::setHeader(const std::string &key, const std::string &value)
{
	this->_headerStream << key << ": " << value << "\r\n";
}

void Response::setBody(const std::string &body, const std::string &type)
{
	this->_body = body;
	setHeader("Content-Type", type);
	setHeader("Content-Length", toString(body.size()));
}

std::string Response::build()
{
	std::ostringstream res;
	res << this->_statusLine;
	res << "Date: " << setDate();
	res << "Server: MyWebServ/1.0\r\n";
	res << "Connection: " << setConnection(this->_request);
	res << _headerStream.str();
	res << "\r\n";
	res << this->_body;
	return res.str();
}

// std::string Response::buildHeaders()
// {
// 	this->_headersFinal << this->_statusLine;
// 	this->_headersFinal << "Date: " << setDate();
// 	this->_headersFinal << "Server: MyWebServ/1.0\r\n";
// 	this->_headersFinal << "Connection: " << setConnection(this->_request);
// 	this->_headersFinal << this->_headerStream.str();
// 	this->_headersFinal << "\r\n";
// }

void Response::sendTo()
{
	//this->buildHeaders();
	std::string full = this->build();
	send(this->_clientFd, full.c_str(), full.size(), MSG_NOSIGNAL);
	std::cout << GREEN "[<] Sent Response:\n" << full.c_str() << RESET << std::endl;
}
//------------------------- ERRORS DURING PARSING -------------------------//

//need to make it dynamic with an explanation sentance
std::string	generateDefaultErrorPage(int code)
{
	std::ostringstream body;
	body << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\" data-theme=\"light\">\n"
		<< "<head>\n"
		<< "<meta charset=\"UTF-8\">\n"
		<< "<title>" << code << " " << statusCodeResponse(code) << "</title>\n"
		<< "<link rel=\"stylesheet\"  href=\"/siteUtils/sidebar.css\">\n"
		<< "<style>\n"
		<< "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }\n"
		<< "h1 { font-size: 48px; color: #d9534f; }\n"
		<< "</style>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "<h1>" << code << " " << statusCodeResponse(code) << "</h1>\n"
		<< "<p>The server encountered an error while processing your request.</p>\n"
		<< "<div id=\"sidebar-container\"></div>\n"
		<< "<script src=\"/siteUtils/cookies.js\"></script>\n"
		<< "</body>\n"
		<< "</html>\n";
	return body.str();
}

std::string readFileToString(const std::string &path)
{
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Cannot open file: " + path);
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

// ─── ERROR RESPONSE ────────────────────────────────────────────────
bool	Response::errorResponse()
{
	if (this->_request.statusCode < 400)
		return false;
	
	this->setStatusCode();
	this->setStatusLine();

	const std::map<int, std::string> errorPages = this->_server.getErrorPages();
	std::map<int, std::string>::const_iterator it = errorPages.find(this->_code);
	std::string body;

	if (it != errorPages.end())
	{
		try
		{
			std::string errorPagePath = it->second;
			std::string root = (this->_server.getData().find("root") != this->_server.getData().end())
							? this->_server.getData().find("root")->second : "";
			if (!errorPagePath.empty() && errorPagePath[0] == '/')
					errorPagePath = "." + root + errorPagePath;
			body = readFileToString(errorPagePath);
		}
		catch (const std::exception &e)
		{
			std::cerr << RED "[!] Failed to read custom error page: " 
						<< e.what() << RESET << std::endl;
			body = generateDefaultErrorPage(this->_code);
		}
	}
	else
	{
		body = generateDefaultErrorPage(this->_code);
	}

	std::vector<char> tmpBody(body.begin(), body.end()); // cookies
	modifyFile(tmpBody, this->_request); // cookies
	body = std::string(tmpBody.begin(), tmpBody.end()); //cookies

	//this->build();
	this->setBody(body, "text/html");
	this->sendTo();

	// std::string headers = buildHeaders(server, resp, request, body.size(), "text/html", true);
	// send(client_fd, headers.c_str(), headers.size(), MSG_NOSIGNAL);
	// send(client_fd, body.c_str(), body.size(), MSG_NOSIGNAL);

	// std::cout << GREEN "[<] Sent Error " << this->_code 
	// 			<< " for " << this->_request.path << RESET << std::endl;
	// std::cout << GREEN "[<] Sent ERROR Page: " << this->_request.path
	// 		<< " (" << body.size() << " bytes)" << RESET << std::endl;
	return true;
}

// ─── AUTOINDEX RESPONSE ───────────────────────────────────────────
bool	Response::autoIndexResponse()
{
	if (this->_request.autoIndexFile.empty())
		return false;

	this->_code = 200;
	this->setStatusLine();

	std::string	body = this->_request.autoIndexFile;
	std::vector<char> tmpBody(body.begin(), body.end()); // cookies
	modifyFile(tmpBody, this->_request); // cookies
	body = std::string(tmpBody.begin(), tmpBody.end()); //cookies

	this->setBody(body, "text/html");
	this->sendTo();

	// std::cout << GREEN "[<] Sent Error " << this->_code 
	// 			<< " for " << this->_request.path << RESET << std::endl;
	// std::cout << GREEN "[<] Sent AutoIndexFile: " << this->_request.path
	// 		<< " (" << body.size() << " bytes)" << RESET << std::endl;

	return true;
}

// ─── REDIRECT RESPONSE (3XX) ──────────────────────────────────────
bool	Response::redirectResponse()
{
	if (this->_request.statusCode < 300 || this->_request.statusCode > 399)
		return false;

	this->setStatusCode();
	this->setStatusLine();

	this->setHeader("Location", this->_request.url + '/');

	std::ostringstream	body;
	body << "<!DOCTYPE html><html><head><title>" << this->_code << " "
		<< statusCodeResponse(this->_code)
		<< "</title></head><body><h1>" << this->_code << " "
		<< statusCodeResponse(this->_code)
		<< "</h1><p>Redirecting to <a href=\"" << this->_request.url + "/" << "\">"
		<< this->_request.url + "/" << "</a></p></body></html>";

	if (this->_code != 304)
		this->setBody(body.str(), "text/html");
	else // will create a problem here because Content-Lenght already in setBody()
		setHeader("Content-Length", "0\r\n\r\n");
	this->sendTo();

	return true;
}

// ─── CGI EXECUTION ────────────────────────────────────────────────
bool	Response::cgiResponse()
{
	if (!this->_request.isCgi)
		return false;

	std::cout << BLUE "[CGI] Executing script: " << this->_request.path << RESET << std::endl;
	try
	{
		CGI cgi;

		std::string result = cgi.executeCgi(this->_request, this->_server, this->_clientFd);
		send(this->_clientFd, result.c_str(), result.size(), MSG_NOSIGNAL);
		//need to protect send
	}
	catch (const std::exception &e)
	{
		std::cerr << RED "[CGI ERROR] " << e.what() << RESET << std::endl;
	}
	return true;
}

// ─── POST HANDLER ────────────────────────────────────────────────
bool		Response::postMethodResponse()
{
	if (this->_request.method != "POST")
		return false;
}

void sendResponse(int client_fd, const HttpRequest &req, Server &server)
{
	Response	resp(client_fd, req, server);

	if (resp.errorResponse())
		return;
	if (resp.autoIndexResponse())
		return;
	if (resp.redirectResponse())
		return;
	if (resp.cgiResponse())
		return;
	if (resp.postMethodResponse())
		return;
	// handleFileResponse(client_fd, req, server);
}
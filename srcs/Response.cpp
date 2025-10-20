#include "Response.hpp"
#include "Server.hpp"
#include "CGI.hpp"

//Operators----------------------------------------------------------------

Response &Response::operator=(Response const &src)
{
	if (this != &src)
	{
		_request = src._request;
		_server = src._server;
		_clientFd = src._clientFd;
		_code = src._code;
		_statusLine = src._statusLine;
	}
	return (*this);
}

//Constructor/Destructors--------------------------------------------------

Response::Response(int clientFd, HttpRequest &request, Server &server): _server(server), _request(request), _clientFd(clientFd)
{}

Response::Response(Response const &src) : _server(src._server), _request(src._request), _clientFd(src._clientFd)
{
	*this = src;
	return ;
}

Response::~Response(void)
{}

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
	{
		this->_code = this->_request.statusCode;
		return;
	}
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

void Response::appendCookies(std::ostringstream &res)
{
	if (this->_server.getModified() < 0)
		return;

	Cookies &cookie = this->_server.getCookies()[this->_server.getModified()];

	if (!cookie.getPrevId().empty())
	{
		res << "Set-Cookie: id=" << cookie.getPrevId()
			<< "; Path=/; Max-Age=0; HttpOnly\r\n";
		cookie.setPrevId("");
	}

	if (!cookie.getPrevAuthToken().empty())
	{
		res << "Set-Cookie: auth_token=" << cookie.getPrevAuthToken()
			<< "; Path=/; Max-Age=0\r\n";
		cookie.setPrevAuthToken("");
	}

	if (cookie.getModified() >= 0)
	{
		std::vector<std::string> vect = cookie.getOutputData();
		if (cookie.getModified() < static_cast<int>(vect.size()))
			res << "Set-Cookie: " << vect[cookie.getModified()] << "\r\n";
	}

	this->_server.setModified(-1);
	cookie.setModified(-1);
}


std::string Response::build()
{
	std::ostringstream res;
	res << this->_statusLine;
	res << "Date: " << setDate();
	res << "Server: MyWebServ/1.0\r\n";
	res << "Connection: " << setConnection(this->_request);
	res << _headerStream.str();
	appendCookies(res);
	res << "\r\n";
	//std::cout << GREEN "[<] Sent Response:\n" << res.str() << RESET << std::endl;
	res << this->_body;
	return res.str();
}

void Response::sendTo()
{
	std::string full = this->build();
	ssize_t ret = send(this->_clientFd, full.c_str(), full.size(), MSG_NOSIGNAL);
	if (ret <= 0)
	{
		std::cerr << RED "[sendTo] send() failed (client may have disconnected)" << RESET << std::endl;
		close(this->_clientFd);
		return;
	}
	std::cout << GREEN "[<] Sent Response:\n" << full.c_str() << RESET << std::endl;
}
//------------------------- ERRORS DURING PARSING -------------------------//

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

// ----- ERROR RESPONSE ----------------------------------------------------
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

	this->setBody(body, "text/html");
	this->sendTo();

	return true;
}

// ----- AUTOINDEX RESPONSE ----------------------------------------------------------------------─
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

	return true;
}

// ----- REDIRECT RESPONSE (3XX) ------------------------------------------------------------──
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
	else
		setHeader("Content-Length", "0\r\n\r\n");
	this->sendTo();

	return true;
}

// ----- CGI EXECUTION ----------------------------------------------------
bool	Response::cgiResponse(Context &context)
{
	if (!this->_request.isCgi)
		return false;

	std::cout << BLUE "[CGI] Executing script: " << this->_request.path << RESET << std::endl;
	try
	{
		CGI cgi(this->_request);

		cgi.setCgiInfos(this->_request, this->_server);

		if (cgi.getCgiType() == UNKNOWN)
		{
			this->_request.statusCode = 401;
			throw std::runtime_error("[CGI ERROR] Unsupported CGI extension: " + cgi.getExtension());
		}

		if (cgi.checkAccess() <= 0)
			throw std::runtime_error("[CGI ERROR] CGI file not accessible: " + cgi.getPath());

		std::string result = cgi.executeCgi(this->_request, this->_server, this->_clientFd, context);
		if (this->_server.getFork())
			throw std::runtime_error("fail");
		size_t ret = send(this->_clientFd, result.c_str(), result.size(), MSG_NOSIGNAL);
		if (ret <= 0)
		{
			std::cerr << RED "[sendTo] send() failed (client may have disconnected)" << RESET << std::endl;
			close(this->_clientFd);
		}
	}
	catch (const std::exception &e)
	{
		std::string msg = e.what();
		std::cerr << RED << msg << RESET << std::endl;
		this->errorResponse();
	}
	return true;
}

// ----- POST HANDLER ----------------------------------------------------

std::string Response::buildPostConfirmation()
{
	std::ostringstream body;
	body << "<!DOCTYPE html>\n<html><head><title>POST Result</title></head><body>";
	body << "<h1 style='color:green;'>POST request successful!</h1>";
	body << "<p>Method: " << this->_request.method << "</p>";
	body << "<p>Path: " << this->_request.path << "</p>";
	body << "</body></html>";
	return body.str();
}

bool		Response::postMethodResponse()
{
	if (this->_request.method != "POST")
		return false;

	if (this->_request.statusCode == 201 || this->_request.statusCode == 0)
	{
		this->setStatusCode();
		this->setStatusLine();
		std::string	body;
		std::string	type = "text/html";
		if (this->_request.jsonResponse.empty()) // File created, showing confirmation page
			body = buildPostConfirmation();
		else //no page change, only confirmation
		{
			body = this->_request.jsonResponse;
			type = "application/json";
		}
		this->setBody(body, type);
		this->sendTo();
		return true;
	}

	if (this->_request.statusCode == 205)
	{
		if (this->_request.autoIndexFile.empty())
		{
			this->_code = 200;
			this->setStatusLine();

			std::ostringstream page;
			page << "<!DOCTYPE html>\n"
				<< "<html lang=\"en\" data-theme=\"light\">\n"
				<< "<head>\n"
				<< "<meta charset=\"UTF-8\">\n"
				<< "<title>File Updated</title>\n"
				<< "<link rel=\"stylesheet\" href=\"/styles.css\">\n"
				<< "<link rel=\"stylesheet\" href=\"/siteUtils/sidebar.css\">\n"
				<< "<style>\n"
				<< "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }\n"
				<< ".message { color: green; font-size: 1.5em; font-weight: bold; }\n"
				<< ".button { display: inline-block; margin-top: 2em; padding: 0.7em 1.2em; "
				<< "background-color: #007BFF; color: white; text-decoration: none; border-radius: 8px; }\n"
				<< ".button:hover { background-color: #0056b3; }\n"
				<< "</style>\n"
				<< "</head>\n"
				<< "<body>\n"
				<< "<h1 class=\"message\">✅ Your file already exists and has been updated.</h1>\n"
				<< "<p>You can go back to the uploads directory to verify it.</p>\n"
				<< "<div class=\"button-container\">\n"
				<< "<a class=\"button\" href=\"/\">🏠 Back to Home Page</a>\n"
				<< "</div>\n"
				<< "<div id=\"sidebar-container\"></div>\n"
				<< "<script src=\"/siteUtils/cookies.js\"></script>\n"
				<< "</body>\n"
				<< "</html>\n";

			std::string body = page.str();
			std::vector<char> tmpBody(body.begin(), body.end()); // cookies
			modifyFile(tmpBody, this->_request); // cookies
			body = std::string(tmpBody.begin(), tmpBody.end()); //cookies

			this->setBody(body, "text/html");
			this->sendTo();

			return true;
		}

		this->_code = 303;
		this->setStatusLine();

		std::cout << BOLD "[DEBUG] " << this->_request.url << RESET << std::endl;
		if (!this->_request.autoIndexFile.empty())
			std::cout << BOLD "[DEBUG] " << this->_request.url << RESET << std::endl;

		this->setHeader("Location", this->_request.url + "/");
		this->setHeader("Content-Length", "0");
		this->sendTo();
		return true;
	}

	return false;
}

// ----- STATIC FILE HANDLER ------------------------------------------------------
bool	Response::fileResponse()
{
	std::ifstream file(this->_request.path.c_str(), std::ios::binary);
	if (!file.is_open() && this->_request.method != "DELETE" && this->_request.statusCode != 100 &&
				!(this->_request.method == "POST" && (this->_request.url == "/register" || this->_request.url == "/login"
				|| this->_request.url == "/logout")) && !(this->_request.method == "GET" && this->_request.url == "/me"))
	{
		this->_code = 500;
		this->setStatusLine();
		std::string body = generateDefaultErrorPage(this->_code);
		this->setBody(body, "text/html");
		this->sendTo();
		return true;
	}

	this->setStatusCode();
	this->setStatusLine();
	std::string	mimeType = getContentType(this->_request.path);
	std::string	body;

	if (!this->_request.jsonResponse.empty())
	{
		body = this->_request.jsonResponse;
		mimeType = "application/json";
	}
	else
	{
		std::ostringstream ss;
		ss << file.rdbuf();
		body = ss.str();
	}

	if (mimeType == "text/html")
	{
		std::vector<char> tmp(body.begin(), body.end());
		modifyFile(tmp, this->_request);
		body.assign(tmp.begin(), tmp.end());
	}

	this->setBody(body, mimeType);
	this->sendTo();
	return true;
}


void sendResponse(int client_fd, HttpRequest &req, Server &server, Context &context)
{
	Response	resp(client_fd, req, server);

	(void)context;
	if (resp.errorResponse())
		return;
	if (resp.autoIndexResponse())
		return;
	if (resp.redirectResponse())
		return;
	if (resp.cgiResponse(context))
		return;
	if (resp.postMethodResponse())
		return;
	resp.fileResponse();
}

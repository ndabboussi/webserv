#include "Server.hpp"
#include "CGI.hpp"

enum	MimeCategory
{
	APPLICATION,
	AUDIO,
	IMAGE,
	MULTIPART,
	TEXT,
	VIDEO,
	VND,
	UNKNOWN
};

MimeCategory	getMimeCategory(const std::string &path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos)
		return UNKNOWN;

	std::string ext = path.substr(dotPos + 1);

	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == "pdf" || ext == "json" || ext == "xml" || ext == "zip" || ext == "xhtml" ||
		ext == "js" || ext == "ogg")
		return APPLICATION;

	if (ext == "mp3" || ext == "wav" || ext == "wma" || ext == "ra")
		return AUDIO;

	if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" || ext == "tiff" ||
		ext == "ico" || ext == "svg" || ext == "djvu")
		return IMAGE;

	if (ext == "form" || ext == "multipart")
		return MULTIPART;

	if (ext == "html" || ext == "htm" || ext == "css" || ext == "txt" || ext == "csv")
		return TEXT;

	if (ext == "mp4" || ext == "mpeg" || ext == "avi" || ext == "wmv" || ext == "flv" || ext == "webm")
		return VIDEO;

	if (ext == "doc" || ext == "docx" || ext == "xls" || ext == "xlsx" ||
		ext == "ppt" || ext == "pptx" || ext == "odt" || ext == "ods" || ext == "odp")
		return VND;

	return UNKNOWN;
}

std::string	getContentType(const std::string &path)
{
	MimeCategory category = getMimeCategory(path);

	switch (category)
	{
		case APPLICATION:
			if (path.find(".pdf") != std::string::npos) return "application/pdf";
			if (path.find(".json") != std::string::npos) return "application/json";
			if (path.find(".xml") != std::string::npos) return "application/xml";
			if (path.find(".zip") != std::string::npos) return "application/zip";
			if (path.find(".js")  != std::string::npos) return "application/javascript";
			return "application/octet-stream";

		case AUDIO:
			if (path.find(".mp3") != std::string::npos) return "audio/mpeg";
			if (path.find(".wav") != std::string::npos) return "audio/x-wav";
			if (path.find(".wma") != std::string::npos) return "audio/x-ms-wma";
			return "audio/*";

		case IMAGE:
			if (path.find(".jpg")  != std::string::npos || path.find(".jpeg") != std::string::npos)
				return "image/jpeg";
			if (path.find(".png")  != std::string::npos) return "image/png";
			if (path.find(".gif")  != std::string::npos) return "image/gif";
			if (path.find(".svg")  != std::string::npos) return "image/svg+xml";
			if (path.find(".ico")  != std::string::npos) return "image/x-icon";
			return "image/*";

		case MULTIPART:
			return "multipart/form-data";

		case TEXT:
			if (path.find(".html") != std::string::npos || path.find(".htm") != std::string::npos)
				return "text/html";
			if (path.find(".css") != std::string::npos) return "text/css";
			if (path.find(".csv") != std::string::npos) return "text/csv";
			return "text/plain";

		case VIDEO:
			if (path.find(".mp4") != std::string::npos) return "video/mp4";
			if (path.find(".avi") != std::string::npos) return "video/x-msvideo";
			if (path.find(".flv") != std::string::npos) return "video/x-flv";
			if (path.find(".webm")!= std::string::npos) return "video/webm";
			return "video/*";

		case VND:
			if (path.find(".docx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
			if (path.find(".doc")  != std::string::npos) return "application/msword";
			if (path.find(".xls")  != std::string::npos) return "application/vnd.ms-excel";
			if (path.find(".xlsx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
			if (path.find(".pptx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
			if (path.find(".ppt")  != std::string::npos) return "application/vnd.ms-powerpoint";
			return "application/vnd";

		default:
			return "application/octet-stream";
	}
}

static std::string  statusCodeResponse(int code)
{
	switch (code)
	{
		//informational responses
		case 100: return "Continue";
		case 101: return "Switching Protocols";

		//succesful responses
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";//
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";//

		//redirection messages
		case 300: return "Multiple Choices"; //dont handle
		case 301: return "Moved Permanently";
		case 302: return "Found"; // handle ?
		case 303: return "See Other";
		case 304: return "Not Modified"; // handle ?
		case 305: return "Use Proxy"; //dont handle
		case 307: return "Temporary Redirect"; //handle ?
	//301/302/307 → you must send a Location header

		//client error responses
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable"; 	
		case 407: return "Proxy Authentication Required"; 	
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 426: return "Upgrade Required";

		//NGINX
		case 444: return "No Response";
		case 494: return "Request Header Too Large";
		case 465: return "SSL Certificate Error";
		case 496: return "SSL Certificate Required";
		case 497: return "HTTP Request Sent To HTTPs Port";
		case 433: return "Client Closed Request";

		//server error responses
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable ";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";

		default:  return "Error";
	}
}


//------------------------- HEADERS UTILS -------------------------//

std::string	toString(size_t value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

void	setStatusCode(HttpRequest const &request, HttpResponse &response)
{
	if (request.statusCode >= 400)
		return;
	if (request.method == "DELETE")
	{
		response.code = 204; //specifying that we won't send body
		return;
	}
	else if (request.statusCode == 0)
	{
		response.code = 200;
		return;
	}
	else
		response.code = request.statusCode;
}

void	setStatusLine(HttpResponse &response)
{
	std::ostringstream line;
	line << "HTTP/1.1 " << response.code << " " << statusCodeResponse(response.code) << "\r\n";
	response.statusLine = line.str();
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

//------------------------- POST confirmation page -------------------------//

std::string buildPostConfirmation(const HttpRequest &request)
{
	std::ostringstream body;
	body << "<!DOCTYPE html>\n<html><head><title>POST Result</title></head><body>";
	body << "<h1 style='color:green;'>POST request successful!</h1>";
	body << "<p>Method: " << request.method << "</p>";
	body << "<p>Path: " << request.path << "</p>";
	body << "</body></html>";
	return body.str();
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

//------------------------- MAIN SENDRESPONSE() FUNCTION -------------------------//

std::string buildHeaders(Server &server, HttpResponse &resp, const HttpRequest &req, size_t bodySize, const std::string &contentType, bool sendBody)
{
	std::ostringstream headers;

	headers << resp.statusLine;
	headers << "Date: " << setDate();
	headers << "Server: MyWebServ/1.0\r\n";
	headers << "Connection: " << setConnection(req);
	if (server.getModified() >= 0)
	{
		Cookies &cookie = server.getCookies()[server.getModified()];
		if (!cookie.getPrevId().empty())
		{
			headers << "Set-Cookie: " << "id=" + cookie.getPrevId() + "; Path=/; Max-Age=0; HttpOnly" << "\r\n";
			cookie.setPrevId("");
		}
		headers << "Set-Cookie: " << cookie.getOutputData()[cookie.getModified()] << "\r\n";
		server.setModified(-1);
		cookie.setModified(-1);
	}

	// Add headers & body if allowed
	if (sendBody)
	{
		headers << "Content-Type: " << contentType << "\r\n";
		headers << "Content-Length: " << bodySize << "\r\n";
	}
	headers << "\r\n";
	return headers.str();
}

void sendRedirectResponse(int client_fd, int code, const std::string &location, const HttpRequest &req)
{
	HttpResponse resp;
	resp.code = code;
	setStatusLine(resp);

	std::ostringstream body;
	body << "<!DOCTYPE html>\n<html><head><title>"
			<< code << " " << statusCodeResponse(code) 
			<< "</title></head><body>"
			<< "<h1>" << code << " " << statusCodeResponse(code) << "</h1>"
			<< "<p>See: <a href=\"" << location + '/' << "\">" << location + '/' << "</a></p>"
			<< "</body></html>";
	std::string bodyStr = body.str();

	std::ostringstream headers;
	headers << resp.statusLine;
	headers << "Date: " << setDate();
	headers << "Server: MyWebServ/1.0\r\n";
	headers << "Connection: " << setConnection(req);
	headers << "Location: " << location + '/' << "\r\n";

	//add body only if not 304(not modified)
	if (code != 304)
	{
		headers << "Content-Type: text/html\r\n";
		headers << "Content-Length: " << bodyStr.size() << "\r\n\r\n";
		headers << bodyStr;
	}
	else
		headers << "Content-Length: 0\r\n\r\n";

	std::string fullResponse = headers.str();

	send(client_fd, fullResponse.c_str(), fullResponse.size(), MSG_NOSIGNAL);

	std::cout << GREEN "[<] Sent Response:\n" << fullResponse.c_str() << RESET << std::endl;
	std::cout << GREEN "[<] Sent Redirect " << code << " to "
				<< location << RESET << std::endl;
}

//------------------------- MAIN SENDRESPONSE() FUNCTION -------------------------//

void	sendResponse(int client_fd, const HttpRequest &request, Server &server)
{
	HttpResponse	resp;

	// Step 1: Check if request already has an error
	if (request.statusCode >= 400)
	{
		resp.code = request.statusCode;
		setStatusLine(resp);

		const std::map<int, std::string> errorPages = server.getErrorPages();
		std::map<int, std::string>::const_iterator it = errorPages.find(resp.code);
		std::string body;

		if (it != errorPages.end())
		{
			try
			{
				std::string errorPagePath = it->second;
				std::string root = (server.getData().find("root") != server.getData().end())
								? server.getData().find("root")->second : "";
				if (!errorPagePath.empty() && errorPagePath[0] == '/')
   					 errorPagePath = "." + root + errorPagePath;
				body = readFileToString(errorPagePath);
			}
			catch (const std::exception &e)
			{
				std::cerr << RED "[!] Failed to read custom error page: " 
							<< e.what() << RESET << std::endl;
				body = generateDefaultErrorPage(resp.code);
			}
		}
		else
		{
			body = generateDefaultErrorPage(resp.code);
		}

		std::vector<char> tmpBody(body.begin(), body.end()); // cookies
		modifyFile(tmpBody, request); // cookies
		body = std::string(tmpBody.begin(), tmpBody.end()); //cookies

		std::string headers = buildHeaders(server, resp, request, body.size(), "text/html", true);
		send(client_fd, headers.c_str(), headers.size(), MSG_NOSIGNAL);
		send(client_fd, body.c_str(), body.size(), MSG_NOSIGNAL);

		std::cout << GREEN "[<] Sent Error " << resp.code 
					<< " for " << request.path << RESET << std::endl;
		std::cout << GREEN "[<] Sent ERROR Page: " << request.path
				<< " (" << body.size() << " bytes)" << RESET << std::endl;
		return;
	}
	
	// Step 2: CGI (skip building full response here)
	if (request.isCgi)
	{
		std::cout << BLUE "[CGI] Executing script: " << request.path << RESET << std::endl;
	
		CGI cgi;
		std::string response = cgi.executeCgi(request, server);
		send(client_fd, response.c_str(), response.size(), 0);
		return;
	}

	// Step 3: Autoindex case
	else if (!request.autoIndexFile.empty())
	{
		resp.code = 200;
		setStatusLine(resp);
		std::string body = request.autoIndexFile;
		std::vector<char> tmpBody(body.begin(), body.end()); // cookies
		modifyFile(tmpBody, request); // cookies
		body = std::string(tmpBody.begin(), tmpBody.end()); //cookies
		std::string	headers = buildHeaders(server, resp, request, body.size(), "text/html", true);
		send(client_fd, headers.c_str(), headers.size(), MSG_NOSIGNAL);
		send(client_fd, body.c_str(), body.size(), MSG_NOSIGNAL);
		std::cout << GREEN "[<] Sent Response:\n" << headers.c_str() << RESET << std::endl;
		std::cout << GREEN "[<] Sent AutoIndexFile: " << request.path
				<< " (" << body.size() << " bytes)" << RESET << std::endl;
		return;
	}

	if (request.statusCode >= 300 && request.statusCode <= 399)
	{
		sendRedirectResponse(client_fd, request.statusCode, request.url, request);
		return;
	}

	std::ifstream file(request.path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		resp.code = 500;
		setStatusLine(resp);
		std::string body = generateDefaultErrorPage(resp.code);
		std::string headers = buildHeaders(server, resp, request, body.size(), "text/html", true);

		send(client_fd, headers.c_str(), headers.size(), MSG_NOSIGNAL);
		send(client_fd, body.c_str(), body.size(), MSG_NOSIGNAL);
		std::cout << GREEN "[<] Sent Response:\n" << headers.c_str() << RESET << std::endl;
		std::cout << GREEN "[<] Sent file: " << request.path
				<< " (" << body.size() << " bytes)" << RESET << std::endl;
		return;
	}

	setStatusCode(request, resp);
	setStatusLine(resp);

	if (request.method == "POST")
	{
		if (resp.code == 201)// File created, showing confirmation page (must be 201 normally, change when implemented in parsing)
		{
			std::string body = buildPostConfirmation(request);
			std::string	headers = buildHeaders(server, resp, request, body.size(), "text/html", true);

			send(client_fd, headers.c_str(), headers.size(), MSG_NOSIGNAL);
			send(client_fd, body.c_str(), body.size(), MSG_NOSIGNAL);
			std::cout << GREEN "[<] Sent Response:\n" << headers.c_str() << RESET << std::endl;
			std::cout << GREEN "[<] Sent POST confirmation page: " << request.path
					<< " (" << body.size() << " bytes)" << RESET << std::endl;
			return;
		}
		else // File already existed, redirect to where to find it
		{
			resp.code = 303;
			setStatusLine(resp);

			std::string location = request.url;
			std::ostringstream headers;
			headers << resp.statusLine;
			headers << "Date: " << setDate();
			headers << "Server: MyWebServ/1.0\r\n";
			headers << "Location: " << location + '/' << "\r\n";
			headers << "Content-Length: 0\r\n";
			headers << "Connection: " << setConnection(request) << "\r\n\r\n";
			
			send(client_fd, headers.str().c_str(), headers.str().size(), MSG_NOSIGNAL);
			std::cout << GREEN "[<] Sent Response:\n" << headers.str().c_str() << RESET << std::endl;
			std::cout << GREEN "[<] Sent 303 See Other to "
						<< location << RESET << std::endl;
			return;
		}
	}

	std::string	mimeType = getContentType(request.path);
	std::vector<char> fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	if ((resp.code >= 100 && resp.code < 200) || resp.code == 204 || resp.code == 304)
	{
		std::string headers = buildHeaders(server, resp, request, 0, mimeType, false);
		send(client_fd, headers.c_str(), headers.size(), MSG_NOSIGNAL);
		std::cout << RED "[<] Sent Response:\n" << headers.c_str() << RESET << std::endl;
		std::cout << GREEN "[<] Sent 204 No Content for " << request.path << RESET << std::endl;
		return;
	}
	if (mimeType == "text/html")
		modifyFile(fileContent, request);// cookies
	std::string headers = buildHeaders(server, resp, request, fileContent.size(), mimeType, true);
	
	send(client_fd, headers.c_str(), headers.size(), MSG_NOSIGNAL);
	send(client_fd, fileContent.data(), fileContent.size(), MSG_NOSIGNAL);
	std::cout << GREEN "[<] Sent Response:\n" << headers.c_str() << RESET << std::endl;
	std::cout << GREEN "[<] Sent file: " << request.path
			<< " (" << fileContent.size() << " bytes)" << RESET << std::endl;
}

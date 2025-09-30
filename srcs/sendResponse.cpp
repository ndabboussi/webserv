#include <Server.hpp>

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
		case 202: return "Accepted";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";

		//redirection messages
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found"; 	
		case 303: return "See Other";	
		case 304: return "Not Modified"; 	
		case 305: return "Use Proxy"; 	
		case 307: return "Temporary Redirect";

		//client error responses
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Bad Request";
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

std::string	generateDefaultErrorPage(int code)
{
	std::ostringstream body;
	body << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "<meta charset=\"UTF-8\">\n"
		<< "<title>" << code << " " << statusCodeResponse(code) << "</title>\n"
		<< "<style>\n"
		<< "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }\n"
		<< "h1 { font-size: 48px; color: #d9534f; }\n"
		<< "p { font-size: 18px; color: #333; }\n"
		<< "</style>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "<h1>" << code << " " << statusCodeResponse(code) << "</h1>\n"
		<< "<p>The server encountered an error while processing your request.</p>\n"
		<< "</body>\n"
		<< "</html>\n";
	return body.str();
}

void	buildErrorBody(int client_fd, int code)
{
	std::string	statusMessage = statusCodeResponse(code);
	std::string body = generateDefaultErrorPage(code);

	std::ostringstream response;
	response << "HTTP/1.1 " << code << " " << statusMessage << "\r\n";
	response << "Content-Type: text/html\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Connection: close\r\n\r\n";
	response << body;
	std::string resp = response.str();
	send(client_fd, resp.c_str(), resp.size(), 0);
	std::cout << GREEN "[<] Sent Response:\n" << resp.c_str() << RESET << std::endl;
	return;
}

// static bool statusAllowsBody(int status)
// {
// 	if ((status >= 100 && status < 200) || status == 204 || status == 304)
// 		return false;
// 	return true;
// }

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

// std::string    setConnection(HttpRequest const &request)
// {
// 	std::string	connect;
// 	if(request.connection == "keep-alive")
// 		connect = "keep-alive\r\n";
// 	else
// 		connect = "close";
// 	return connect;
// }

std::string	setContentType(HttpRequest const &request)
{
	std::string	contentType = getContentType(request.path);
	contentType.append("\r\n");
	return contentType;
}

std::string	setContentLenght(size_t contentLength)
{
	std::ostringstream	oss;
	oss << contentLength;
	std::string	lenght = oss.str();
	lenght.append("\r\n");
	return lenght;
}

void setHeaders(HttpResponse &response, const HttpRequest &request, size_t contentLength)
{
	response.headers["Date: "] = setDate();
	response.headers["Server: "] = "MyWebServ/1.0";  // tu peux mettre ton nom de projet
	//response.headers["Connection: "] = setConnection(request);
	response.headers["Content-Type: "] = setContentType(request);
	response.headers["Content-Length: "] = setContentLenght(contentLength);
}

void	setStatusCode(HttpRequest const &request, HttpResponse &response)
{
	if (request.method == "POST")
		response.code = 204;
	else if (request.method == "DELETE")
		response.code = 204; //specifying that we won't send body
	response.code = 200;
}

void sendResponse(int client_fd, const HttpRequest &request)
{
	if (request.error)
	{
		buildErrorBody(client_fd, request.error);
		return;
	}

	struct stat fileStat;
	if (stat(request.path.c_str(), &fileStat) == -1 || !S_ISREG(fileStat.st_mode))
	{
		buildErrorBody(client_fd, 404);
		return;
	}

	// std::ifstream file(request.path.c_str(), std::ios::binary);
	// std::vector<char> fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// std::string body(fileContent.begin(), fileContent.end());

	// HttpResponse response;
	// setStatusCode(request, response);
	// setStatusLine(response);
	// //setHeaders(response);
	
	// setHeaders(response, request, body.size());

	// std::ostringstream fullResponse;
	// fullResponse << response.statusLine;

	// for (std::map<std::string, std::string>::const_iterator it = response.headers.begin();
	// 		it != response.headers.end(); ++it)
	// {
	// 	fullResponse << it->first << ": " << it->second << "\r\n";
	// }
	// fullResponse << "\r\n"; // headers ends
	// fullResponse << body;

	// std::string responseStr = fullResponse.str();
	// send(client_fd, responseStr.c_str(), responseStr.size(), 0);


	std::ifstream file(request.path.c_str(), std::ios::binary);
	std::vector<char> fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: " << getContentType(request.path) << "\r\n";
	response << "Content-Length: " << fileContent.size() << "\r\n";
	response << "Connection: close\r\n\r\n";

	std::string headers = response.str();
	send(client_fd, headers.c_str(), headers.size(), 0);
	send(client_fd, fileContent.data(), fileContent.size(), 0);

	//std::cout << GREEN "[<] Sent Response:\n" << fullResponse.str() << RESET;
	std::cout << GREEN "[<] Sent Response:\n" << headers.c_str() << RESET;
	std::cout << GREEN "[<] Sent file: " << request.path
				<< " (" << fileContent.size() << " bytes)" << RESET << std::endl;
}


//For 4xx and 5xx responses: send a small HTML page explaining the error.
//For 3xx responses (redirections): you usually do not need a body, but you should send a Location: <url> header.
//For 204 (No Content): you must not send a body at all.


//redirection 300
//autoindex !!!!!!
//changer URL par server_name replace local_host
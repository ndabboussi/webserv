#include "Server.hpp"
#include "CGI.hpp"

MimeCategory	getMimeCategory(const std::string &path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos)
		return UNKNOWN_MIME;

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

	return UNKNOWN_MIME;
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

std::string  statusCodeResponse(int code)
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
		case 205: return "Reset Content";//no body
		case 206: return "Partial Content";//

		//redirection messages
		case 300: return "Multiple Choices"; //dont handle
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 305: return "Use Proxy";
		case 307: return "Temporary Redirect";
	//301/302/307 → must send a Location header

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

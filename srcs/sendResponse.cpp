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

void sendResponse(int client_fd, const HttpRequest &request)
{
	if (request.path.empty())
	{
		std::string body = "<h1>Hello from webserv!</h1>";
		std::ostringstream response;
		response << "HTTP/1.1 200 OK\r\n";
		response << "Content-Type: text/html\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Connection: close\r\n\r\n";
		response << body;
		std::string resp = response.str();
		send(client_fd, resp.c_str(), resp.size(), 0);
		return;
	}

	struct stat fileStat;
	if (stat(request.path.c_str(), &fileStat) == -1 || !S_ISREG(fileStat.st_mode))
	{
		std::string body = "<h1>404 Not Found</h1>";
		std::ostringstream response;
		response << "HTTP/1.1 404 Not Found\r\n";
		response << "Content-Type: text/html\r\n";
		response << "Content-Length: " << body.size() << "\r\n";
		response << "Connection: close\r\n\r\n";
		response << body;
		std::string resp = response.str();
		send(client_fd, resp.c_str(), resp.size(), 0);
		return;
	}
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

	std::cout << GREEN "[<] Sent file: " << request.path
				<< " (" << fileContent.size() << " bytes)" << RESET << std::endl;
}

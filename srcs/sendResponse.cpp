#include <Server.hpp>

std::string getContentType(const std::string &path)
{
	if (path.find(".html") != std::string::npos) return "text/html";
	if (path.find(".jpg")  != std::string::npos) return "image/jpeg";
	if (path.find(".jpeg") != std::string::npos) return "image/jpeg";
	if (path.find(".png")  != std::string::npos) return "image/png";
	return "not/found";
}

void sendResponse(int client_fd, const HttpRequest &request)
{
	std::string filePath = request.path;
	if (filePath.empty())
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
	if (stat(filePath.c_str(), &fileStat) == -1 || !S_ISREG(fileStat.st_mode))
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
	std::ifstream file(filePath.c_str(), std::ios::binary);
	std::vector<char> fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: " << getContentType(filePath) << "\r\n";
	response << "Content-Length: " << fileContent.size() << "\r\n";
	response << "Connection: close\r\n\r\n";

	std::string headers = response.str();
	send(client_fd, headers.c_str(), headers.size(), 0);
	send(client_fd, fileContent.data(), fileContent.size(), 0);

	std::cout << GREEN "[<] Sent file: " << filePath
				<< " (" << fileContent.size() << " bytes)" << RESET << std::endl;
}

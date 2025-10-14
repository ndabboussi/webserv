#include "Server.hpp"
#define BUFSIZE 10

//------------------------------------ CHUNKS -------------------------------------//

static int readLine(int &client_fd, std::string &line)
{
	int byteRead = 0;
	char c = 0;

	line.clear();
	while (c != '\n')
	{
		int res = read(client_fd, &c, 1);
		if (res < 0)
			return -1;
		else if (!res)
			return byteRead;
		byteRead += res;
		line += c;
	}
	return byteRead;
}

static void deleteChunkSize(std::string data, std::string &src)
{
	std::istringstream  before(data);
	std::string			buffer;
	long long			size;
	std::vector<char>	buff;
	char				crlf[2];
	size_t pos = src.find("\r\n\r\n") + 4;
	
	while (std::getline(before, buffer))
	{
		if (!buffer.empty())
		{
			std::stringstream ss;
			ss << std::hex << buffer;
			ss >> size;
			buff.resize(size);
			src.erase(pos, buffer.size() + 1);
			if (size == 0)
				break ;
			pos += size;
			before.read(buff.data(), size);
			before.read(crlf, 2);
			if (crlf[0] != '\r' && crlf[1] != '\n')
				break ;//error bad request
			src.erase(pos, 2);
		}
	}
}

static int	loadByChunk(std::string &data, std::string afterHeader, const Server &server, int &client_fd)
{
	std::istringstream  before(afterHeader);
	long long			bodySize = 0;
	std::string			buffer;
	std::vector<char>	buff;
	long long			ByteRead;
	long long			size;
	long long			left = 0;
	
	while (std::getline(before, buffer))
	{
		if (!buffer.empty() && buffer[buffer.size() - 1] == '\r')
			buffer.erase(buffer.size() - 1);
		if (!buffer.empty())
		{
			std::stringstream ss;
			ss << std::hex << buffer;
			ss >> size;
			buff.resize(size + 2);
			before.read(buff.data(), size + 2);
			bodySize += before.gcount();
			if (size + 2 > before.gcount())
				left = size + 2 - before.gcount();
		}
	}
	if (bodySize > server.getMaxBodyClientSize())
	{
		std::cerr << RED "Error 413: Entity Too Large" << RESET << std::endl;
		return 413;
	}
	if (left > 0)
	{
		buff.resize(left + 2);
		ByteRead = read(client_fd, buff.data(), left + 2);
		if (ByteRead < 0)
			return -1;
		data.append(buff.data(), ByteRead);
	}
	while ((left = readLine(client_fd, buffer)) > 0)
	{
		data.append(buffer, 0, buffer.size());
		std::stringstream ss;
		ss << std::hex << buffer;
		ss >> size;
		if (size == 0)
			break ;
		buff.resize(size + 2);
		ByteRead = read(client_fd, buff.data(), size + 2);
		if (ByteRead < 0)
			return -1;
		bodySize += ByteRead;
		data.append(buff.data(), ByteRead);
		if (bodySize > server.getMaxBodyClientSize())
		{
			std::cerr << RED "Error 413: Entity Too Large" << RESET << std::endl;
			return 413;
		}
	}
	size_t pos = data.find("\r\n\r\n") + 4;
	deleteChunkSize(data.substr(pos, data.size() - pos), data);
	return 0;
}

//------------------------------------ HANDLE CLIENTS -------------------------------------//

// while (1)
// 	{
// 		// bytes_read = read(client_fd, buffer, sizeof(buffer));
// 		bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);//use MSG_DONTWAIT ?
// 		if (bytes_read < 0)
// 		{
// 			std::cerr << RED "Erreur de lecture depuis le client." << RESET << std::endl;// error 500 ?
// 			return (false);
// 		}
// 		if (bytes_read == 0)
// 			break;
// 		data.append(buffer, bytes_read);

// 		if (!stored_header)
// 		{
// 			size_t end = data.find("\r\n\r\n");
// 			if (end != std::string::npos)
// 			{
// 				stored_header = true;

// 				size_t pos;
// 				if ((pos = data.find("Content-Length:")) != std::string::npos)
// 				{
// 					pos += 15;
// 					content_length = std::atoi(data.c_str() + pos);
// 				}
// 				else if (data.find("Transfer-Encoding: chunked") != std::string::npos)
// 				{
// 					int res = loadByChunk(data, data.substr(end + 4, data.size() - end + 4), servers, client_fd);
// 					if (res)
// 					{
// 						HttpRequest req;
// 						req.statusCode = res;
// 						sendResponse(client_fd, req, servers);
// 						return true;
// 					}
// 				}
// 				if (content_length > 0)
// 				{
// 					size_t	content_start = data.find("\r\n\r\n");
// 					size_t	size = data.size() - (content_start + 4);

// 					while (size < content_length)
// 					{
// 						bytes_read = read(client_fd, buffer, sizeof(buffer));
// 						if (bytes_read <= 0)
// 							break;
// 						data.append(buffer, bytes_read);
// 						size += bytes_read;
// 					}
// 				}
// 				break;
// 			}
// 		}
// 	}


static void	sendErrorAndReturn(std::string errMsg, int error, int client_fd, Server &servers)
{
	std::cerr << RED << errMsg << RESET << std::endl;// error 500 ?
	HttpRequest req;
	req.statusCode = error;
	sendResponse(client_fd, req, servers);
}

bool	handleClient(int client_fd, Server &servers, int serverPort)
{
	static std::string	data;
	char				buffer[BUFSIZE] = {0};
	int					bytes_read;
	static size_t		content_length = 0;
	static size_t		endHeader = 0;
	static int			firstRead = 0;
	static int			chunked = 0;

	if (firstRead == 0)
	{
		bytes_read = recv(client_fd, buffer, sizeof(buffer), MSG_DONTWAIT);//use MSG_DONTWAIT ?
		if (bytes_read < 0)
		{
			sendErrorAndReturn("Erreur de lecture depuis le client.", 500, client_fd, servers);
			return (false);
		}
		if (bytes_read == 0)
		{
			sendErrorAndReturn("Error 400: Bad request.", 400, client_fd, servers);
			return (false);
		}
		data.append(buffer, bytes_read);
		endHeader = data.find("\r\n\r\n");
		if (endHeader == std::string::npos)
			return true;
		firstRead = 1;
		endHeader += 4;
		size_t pos;
		if ((pos = data.find("Content-Length:")) != std::string::npos)
		{
			pos += 15;
			content_length = std::atoi(data.c_str() + pos);
		}
		else if (data.find("Transfer-Encoding: chunked") != std::string::npos)
		{
			chunked = 1;
			return true;
		}
		if (data.size() - endHeader < content_length)
			return true;
		firstRead = 0;
	}
	else
	{
		firstRead = 0;
		if (chunked)
		{
			chunked = 0;
			loadByChunk(data,  data.substr(endHeader, data.size() - endHeader), servers, client_fd);
		}
		else
		{
			int size = content_length - (data.size() - endHeader);
			std::vector<char> buf(size);
			bytes_read = recv(client_fd, buf.data(), size, MSG_DONTWAIT);//use MSG_DONTWAIT ?
			if (bytes_read < 0)
			{
				sendErrorAndReturn("Erreur de lecture depuis le client.", 500, client_fd, servers);
				return (false);
			}
			if (bytes_read == 0)
			{
				sendErrorAndReturn("Error 400: Bad request.", 400, client_fd, servers);
				return (false);
			}
			data.append(buf.data(), bytes_read);
		}
	}
	
	if (data.empty())
		return false;

	std::cout << PINK << data << RESET << std::endl;//logger

	HttpRequest request = parseHttpRequest(data, servers);
	data.clear();
	//debugPrintRequest(request);
	request.serverPort = serverPort;
	if (request.statusCode < 300)
		manageCookies(servers, request);
	sendResponse(client_fd, request, servers);
	return false;
}

#include "Client.hpp"

//Operators----------------------------------------------------------------

Client &Client::operator=(Client const &src)
{
    if (this != &src)
    {
		this->_clientFd = src._clientFd;
		this->_port = src._port;
		this->_checkName = src._checkName;
		this->_indexServer = src._indexServer;
		this->_data = src._data;
		this->_content_length = src._content_length;
		this->_endHeader = src._endHeader;
		this->_firstRead = src._firstRead;
		this->_chunked = src._chunked;
		this->_firstChunk = src._firstChunk;
		this->_bodySize = src._bodySize;
		this->_left = src._left;
		this->_continue = src._continue;
		this->_before = src._before;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Client::Client(void)
{}

Client::Client(int clientFd, size_t indexServer, int port): _clientFd(clientFd), _indexServer(indexServer), _port(port),
		_checkName(0), _content_length(0), _endHeader(0), _firstRead(0), _chunked(0), _firstChunk(0), _continue(0), _bodySize(0), _left(0)
{}

Client::Client(Client const &src) : _clientFd(src._clientFd), _indexServer(src._indexServer), _port(src._port), _checkName(src._checkName),
		_data(src._data), _content_length(src._content_length), _endHeader(src._endHeader), _firstRead(src._firstRead), _chunked(src._chunked),
		_firstChunk(src._firstChunk), _continue(src._continue), _before(src._before), _bodySize(src._bodySize), _left(src._left)
{}

Client::~Client(void)
{}

//GETTERS ---------------------------------------------------------

int	Client::getClientFd(void) const
{
	return this->_clientFd;
}

size_t Client::getIndexServer(void) const
{
	return this->_indexServer;
}

int	Client::getPort(void) const
{
	return this->_port;
}

//SETTERS ---------------------------------------------------------

void	Client::setClientFd(int fd)
{
	this->_clientFd = fd;
}

void	Client::setIndexServer(size_t index)
{
	this->_indexServer = index;
}

void	Client::setPort(int port)
{
	this->_port = port;
}

//Member functions

static void	sendErrorAndReturn(std::string errMsg, int error, int client_fd, Server &servers)
{
	std::cerr << RED << errMsg << RESET << std::endl;// error 500 ?
	HttpRequest req;
	req.isCgi = false;
	req.statusCode = error;
	if (error == 413)
	{
		usleep(300000);
		req.header.insert(std::make_pair("Connection", "close"));
	}
	sendResponse(client_fd, req, servers);
}

static int	deleteChunkSize(std::string data, std::string &src)
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
				return 0;
			pos += size;
			before.read(buff.data(), size);
			before.read(crlf, 2);
			if (crlf[0] != '\r' && crlf[1] != '\n')
				break ;//error bad request
			src.erase(pos, 2);
		}
	}
	return 1;
}

static long long analyseLine(std::string line)
{
	std::istringstream	stream(line);
	std::string			str;
	long long			size;

	if (std::getline(stream, str))
	{
		if (str[str.size() - 1] == '\r')
			str.erase(str.end() - 1);
		std::stringstream ss;
		ss << std::hex << str;
		ss >> size;
		return size;
	}
	return -1;
}

int	Client::loadByChunk(const Server &server)
{
	//std::string			buffer;
	std::vector<char>	buff(5, '\0');
	long long			byteRead;
	long long			size = 0;
	
	if (!this->_left)
	{
		byteRead = recv(this->_clientFd, buff.data(), 4, 0);
		if (byteRead < 0)
			return 500;
		if (byteRead == 0)
			return 400;
		if (byteRead == 2 && buff.data()[0] == '\r' && buff.data()[1] == '\n')
			return 400;
		this->_before += buff.data();
		this->_data.append(buff.data());
		size = analyseLine(this->_before);
		if (size < 0)
			return -1;
		size_t pos = std::string(buff.data()).find('\n');
		if (pos == std::string::npos)
			return -1;
		if (size == 0)
		{
			size_t pos = this->_data.find("\r\n\r\n") + 4;
			deleteChunkSize(this->_data.substr(pos, this->_data.size() - pos), this->_data);
			return 0;
		}
		this->_bodySize += byteRead - static_cast<int>(pos + 1);
		this->_left = size - (byteRead - static_cast<int>(pos + 1));
		if (this->_bodySize > server.getMaxBodyClientSize())
			return 413;
		this->_before.clear();
		return -1;
	}
	else
	{
		buff.resize(this->_left + 2 + 1, '\0');
		byteRead = recv(this->_clientFd, buff.data(), this->_left + 2, 0);
		if (byteRead < 0)
			return 500;
		if (byteRead == 0)
			return 400;
		this->_data.append(buff.data());
		if (this->_data[this->_data.size() - 2] != '\r' || this->_data[this->_data.size() - 1] != '\n')
			return 400;
		this->_bodySize += byteRead - 2;
		if (this->_bodySize > server.getMaxBodyClientSize())
			return 413;
		this->_left = 0;
		return -1;
	}
	return 0;
}

int Client::checkName(Server &server)
{
	if (!this->_checkName && !this->_data.empty() && this->_data.find("Host: ") != std::string::npos)
	{
		std::string str;
		std::istringstream requestStream(this->_data.substr(this->_data.find("Host: ") + 6));
		if (std::getline(requestStream, str))
		{
			size_t pos = (str.find(':') != std::string::npos) ? str.find(':') : str.size();
			std::string name = server.getName();
			str = str.substr(0, pos);
			if (!name.empty() && str != name)
			{
				sendErrorAndReturn("Error 400: Bad request.", 400, this->_clientFd, server);
				return (false);
			}
		}
		this->_checkName = 1;
	}
	return true;
}

bool	Client::handleClient(Server &server)
{
	char				buffer[4096] = {0};
	int					bytes_read;

	if (this->_content_length > server.getMaxBodyClientSize())
	{
		sendErrorAndReturn("Error 413: Entity Too Large", 413, this->_clientFd, server);
		return false;
	}
	if (this->checkName(server) == false)
		return (false);
	if (this->_firstRead == 0)
	{
		bytes_read = recv(this->_clientFd, buffer, sizeof(buffer), 0);//use MSG_DONTWAIT ?
		if (bytes_read < 0)
		{
			sendErrorAndReturn("Erreur de lecture depuis le client.", 500, this->_clientFd, server);
			return (false);
		}
		if (bytes_read == 0)
		{
			sendErrorAndReturn("Error 400: Bad request.", 400, this->_clientFd, server);
			return (false);
		}
		this->_data.append(buffer, bytes_read);
		if (this->checkName(server) == false)
			return (false);
		this->_endHeader = this->_data.find("\r\n\r\n");
		if (this->_endHeader == std::string::npos)
			return true;
		this->_firstRead = 1;
		this->_endHeader += 4;
		size_t pos;
		if ((pos = this->_data.find("Content-Length:")) != std::string::npos)
		{
			pos += 15;
			this->_content_length = std::atoi(this->_data.c_str() + pos);
		}
		else if (this->_data.find("Transfer-Encoding: chunked") != std::string::npos)
		{
			this->_chunked = 1;
			return true;
		}
		if (static_cast<long long>(this->_data.size() - this->_endHeader) < this->_content_length)
			return true;
		this->_firstRead = 0;
	}
	else
	{
		if (this->_chunked)
		{
			if (this->_data.find("Expect: 100-continue") != std::string::npos && !this->_continue)
			{
				this->_continue = 1;
				sendErrorAndReturn("", 100, this->_clientFd, server);
				return true;
			}
			int res = this->loadByChunk(server);
			if (res > 0)
			{
				std::string msg = (res == 500) ? "Erreur de lecture depuis le client." :
					(res == 400) ? "Error 400: Bad request." : "Error 413: Entity Too Large";
				sendErrorAndReturn(msg, res, this->_clientFd, server);
				return false;
			}
			if (res < 0)
				return true;
			this->_chunked = 0;
		}
		else
		{
			int size = this->_content_length - (this->_data.size() - this->_endHeader);
			std::vector<char> buf(size);
			bytes_read = recv(this->_clientFd, buf.data(), size, 0);//use MSG_DONTWAIT ?
			if (bytes_read < 0)
			{
				sendErrorAndReturn("Erreur de lecture depuis le client.", 500, this->_clientFd, server);
				return (false);
			}
			if (bytes_read == 0)
			{
				sendErrorAndReturn("Error 400: Bad request.", 400, this->_clientFd, server);
				return (false);
			}
			this->_data.append(buf.data(), bytes_read);
			if (bytes_read < size)
				return true;
			this->_firstRead = 0;
		}
	}
	
	if (this->_data.empty())
		return false;

	std::cout << PINK << this->_data << RESET << std::endl;//logger
	HttpRequest request = parseHttpRequest(this->_data, server);
	request.serverPort = this->_port;
	if (request.statusCode < 300)
		manageCookies(server, request);
	sendResponse(this->_clientFd, request, server);
	return false;
}
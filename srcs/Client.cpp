#include "Client.hpp"

//Operators----------------------------------------------------------------

Client &Client::operator=(Client const &src)
{
    if (this != &src)
    {
		this->_clientFd = src._clientFd;
		this->_port = src._port;
		this->_indexServer = src._indexServer;
		this->_data = src._data;
		this->_content_length = src._content_length;
		this->_endHeader = src._endHeader;
		this->_firstRead = src._firstRead;
		this->_chunked = src._chunked;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Client::Client(void)
{}

Client::Client(int clientFd, size_t indexServer, int port): _clientFd(clientFd), _indexServer(indexServer), _port(port),
		_content_length(0), _endHeader(0), _firstRead(0), _chunked(0)
{}

Client::Client(Client const &src) : _clientFd(src._clientFd), _indexServer(src._indexServer), _port(src._port),
		_data(src._data), _content_length(src._content_length), _endHeader(src._endHeader), _firstRead(src._firstRead), _chunked(src._chunked)
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
	req.statusCode = error;
	sendResponse(client_fd, req, servers);
}

bool	Client::handleClient(Server &server)
{
	char				buffer[4096] = {0};
	int					bytes_read;

	if (this->_firstRead == 0)
	{
		bytes_read = recv(this->_clientFd, buffer, sizeof(buffer), MSG_DONTWAIT);//use MSG_DONTWAIT ?
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
		else if (this->_data.find("Transfer-Encoding: this->_chunked") != std::string::npos)
		{
			this->_chunked = 1;
			return true;
		}
		if (this->_data.size() - this->_endHeader < this->_content_length)
			return true;
		this->_firstRead = 0;
	}
	else
	{
		this->_firstRead = 0;
		if (this->_chunked)
		{
			this->_chunked = 0;
			//loadByChunk(data,  data.substr(this->_endHeader, data.size() - this->_endHeader), server, this->_clientFd);
		}
		else
		{
			int size = this->_content_length - (this->_data.size() - this->_endHeader);
			std::vector<char> buf(size);
			bytes_read = recv(this->_clientFd, buf.data(), size, MSG_DONTWAIT);//use MSG_DONTWAIT ?
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
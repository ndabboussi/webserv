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
		this->_request = src._request;
		this->_parsed = src._parsed;
		this->_time = src._time;
		this->_checkMths = src._checkMths;
		this->_cgiPid = src._cgiPid;
		this->_cgiOutputFd = src._cgiOutputFd;
		this->_cgiRunning = src._cgiRunning;
		this->_cgiToSend = src._cgiToSend;
		this->_cgiBuffer = src._cgiBuffer;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Client::Client(void)
{}

Client::Client(int clientFd, size_t indexServer, int port): _parsed(false), _clientFd(clientFd), _indexServer(indexServer), _port(port),
		_time((timeval){0, 0}), _checkName(0), _checkMths(0), _content_length(0), _endHeader(0), _firstRead(0), _chunked(0), _firstChunk(0), _continue(0),
		_bodySize(0), _left(0), _cgiPid(-1), _cgiOutputFd(-1), _cgiRunning(false), _cgiToSend(false) 
{}

Client::Client(Client const &src) : _request(src._request), _parsed(src._parsed), _clientFd(src._clientFd), _indexServer(src._indexServer),
		_port(src._port),  _time(src._time), _checkName(src._checkName), _checkMths(src._checkMths), _data(src._data),
		_content_length(src._content_length), _endHeader(src._endHeader), _firstRead(src._firstRead), _chunked(src._chunked),
		_firstChunk(src._firstChunk), _continue(src._continue), _before(src._before), _bodySize(src._bodySize), _left(src._left), 
		_cgiPid(src._cgiPid), _cgiOutputFd(src._cgiOutputFd), _cgiRunning(src._cgiRunning), _cgiToSend(src._cgiToSend), _cgiBuffer(src._cgiBuffer) 
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

pid_t	Client::getCgiPid() const
{
	return this->_cgiPid;
}

int	Client::getCgiOutputFd() const
{
	return this->_cgiOutputFd;
}

bool	Client::isCgiRunning() const
{
	return this->_cgiRunning;
}

bool	Client::isCgiToSend() const
{
	return this->_cgiToSend;
}


std::string	Client::getCgiBuffer() const
{
	return this->_cgiBuffer;	
}

timeval	Client::getTime() const
{
	return this->_time;	
}

int	Client::getParsed(void) const
{
	return this->_parsed;
}

HttpRequest Client::getRequest(void) const
{
	return this->_request;
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

void	Client::setCgiPid(pid_t pid)
{
	this->_cgiPid = pid;
}

void	Client::setCgiOutputFd(int fd)
{
	this->_cgiOutputFd = fd;
}

void	Client::setCgiRunning(bool flag)
{
	this->_cgiRunning = flag;
}

void	Client::setCgiToSend(bool flag)
{
	this->_cgiToSend = flag;
}

void	Client::setCgiBuffer(std::string buffer)
{
	this->_cgiBuffer = buffer;
}

void	Client::setTime(timeval value)
{
	this->_time = value;
}

void Client::setRequest(HttpRequest req)
{
	this->_request = req;
}

//Member functions

void	Client::sendErrorAndReturn(std::string errMsg, int error)
{
	std::cerr << RED << errMsg << RESET << std::endl;
	this->_request.isCgi = false;
	this->_request.statusCode = error;
	this->_parsed = true;
	if (error == 413)
	{
		usleep(300000);
		this->_request.header.insert(std::make_pair("Connection", "close"));
	}
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
			if (str.size() == 0 || str[str.size() - 1] != '\r')
				return (true);
			size_t pos = (str.find(':') != std::string::npos) ? str.find(':') : str.size();
			std::string name = server.getName();
			str = str.substr(0, pos);
			if (!name.empty() && str != name)
			{
				sendErrorAndReturn("Error 400: Bad request", 400);
				return (false);
			}
		}
		this->_checkName = 1;
	}
	return true;
}

void Client::firstRead(Server &server)
{
	char				buffer[4096] = {0};
	int					bytes_read;

	bytes_read = recv(this->_clientFd, buffer, sizeof(buffer), 0);//use MSG_DONTWAIT ?
	if (bytes_read < 0)
	{
		sendErrorAndReturn("Erreur de lecture depuis le client.", 500);
		throw std::exception();
	}
	if (bytes_read == 0)
	{
		sendErrorAndReturn("Error 400: Bad request.", 400);
		throw std::exception();
	}
	this->_data.append(buffer, bytes_read);
	if (this->checkName(server) == 0)
		throw std::exception();
	this->_endHeader = this->_data.find("\r\n\r\n");
	if (this->_endHeader == std::string::npos)
		throw std::exception();
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
		throw std::exception();
	}
	if (static_cast<long long>(this->_data.size() - this->_endHeader) < this->_content_length)
		throw std::exception();
	this->_firstRead = 0;
}

void Client::otherRead(Server &server)
{
	int	bytes_read;

	if (this->_chunked)
	{
		if (this->_data.find("Expect: 100-continue") != std::string::npos && !this->_continue)
		{
			this->_continue = 1;
			sendErrorAndReturn("", 100);
			throw std::exception();
		}
		else if (!this->_continue)
		{
			sendErrorAndReturn("Error 400: Bad request.", 400);
			throw std::exception();
		}
		int res = this->loadByChunk(server);
		if (res > 0)
		{
			std::string msg = (res == 500) ? "Erreur de lecture depuis le client." :
				(res == 400) ? "Error 400: Bad request." : "Error 413: Entity Too Large";
			sendErrorAndReturn(msg, res);
			throw std::exception();
		}
		if (res < 0)
			throw std::exception();
		this->_chunked = 0;
	}
	else
	{
		int size = this->_content_length - (this->_data.size() - this->_endHeader);
		std::vector<char> buf(size);
		bytes_read = recv(this->_clientFd, buf.data(), size, 0);//use MSG_DONTWAIT ?
		if (bytes_read < 0)
		{
			sendErrorAndReturn("Erreur de lecture depuis le client.", 500);
			throw std::exception();
		}
		if (bytes_read == 0)
		{
			sendErrorAndReturn("Error 400: Bad request.", 400);
			throw std::exception();
		}
		this->_data.append(buf.data(), bytes_read);
		if (bytes_read < size)
			throw std::exception();
		this->_firstRead = 0;
	}
}

void	Client::handleClientWrite(Server &server, Context &context)
{
	if (this->_request.statusCode == 100)
	{
		this->_parsed = false;
		this->_time = (timeval){0, 0};
	}
	sendResponse(*this, this->_clientFd, this->_request, server, context);
}

bool	Client::checkTimeOut()
{
	if (this->_time.tv_sec == 0)
		return false;
	struct timeval current;
	gettimeofday(&current, NULL);
	float elapsed = current.tv_sec - this->_time.tv_sec + ((current.tv_usec - this->_time.tv_usec) / 1e6);
	if (elapsed > 5)
	{
		if (!this->_request.isCgi)
			sendErrorAndReturn("Error 408: Request Timeout", 408);
		return true;
	}
	return false;
}

static int checkErrors(std::string method, std::string version)
{
	if (method != "GET" && method != "POST" && method != "DELETE")
		return 501;
	else if (version != "HTTP/1.1")
		return 505;
	return 0;
}

void	Client::checkFirstLine()
{
	std::istringstream			requestStream(this->_data, std::ios::binary);
	std::string					buffer;
	std::vector<std::string>	line;

	if (std::getline(requestStream, buffer))
	{
		if (buffer[buffer.size() - 1] != '\r')
			return ;
		std::istringstream iss(buffer);
		std::string token;

		while (iss >> token)
			line.push_back(token);
		if (line.size() != 3)
		{
			sendErrorAndReturn("Error 400: Bad request", 400);
			return ;
		}
		int res = checkErrors(line[0], line[2]);
		if (res)
		{
			std::string msg = (res == 501) ? "Error 501: Not inplemented" : "Error 505: HTTP Version Not Supported";
			sendErrorAndReturn(msg, 400);
			return ;
		}
	}
	this->_checkMths = 1;
}

void	Client::handleClientRead(Server &server)
{
	if (this->_time.tv_sec == 0)
		gettimeofday(&this->_time, NULL);
	try
	{
		if (this->_content_length > server.getMaxBodyClientSize())
		{
			sendErrorAndReturn("Error 413: Entity Too Large", 413);
			throw std::exception();
		}
		if (this->checkName(server) == false)
		{
			this->_parsed = true;
			throw std::exception();
		}
		if (this->_firstRead == 0)
			this->firstRead(server);
			
		else
			this->otherRead(server);
		
		if (this->_data.empty())
		{
			this->_parsed = true;
			throw std::exception();
		}
	}
	catch(const std::exception& e)
	{
		if (!this->_data.empty() && !this->_checkMths)
			checkFirstLine();
		return ;
	}
	
	std::cout << PINK << this->_data << RESET << std::endl;
	this->_request = parseHttpRequest(this->_data, server);
	this->_request.serverPort = this->_port;
	if (this->_request.statusCode < 300)
	{
		if (this->_data.find("\r\n\r\n") != std::string::npos)
			this->_request.rawBody = std::vector<char>(this->_data.begin() + this->_data.find("\r\n\r\n") + 4, this->_data.end());
		manageCookies(server, this->_request);
	}
	this->_parsed = true;
	return ;
}

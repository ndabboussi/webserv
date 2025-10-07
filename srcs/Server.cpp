#include "Server.hpp"

//Operators----------------------------------------------------------------

Server &Server::operator=(Server const &src)
{
    if (this != &src)
    {
        Location::operator=(src);
        this->_port = src._port;
        this->_name = src._name;
        this->_maxClientBodySize = src._maxClientBodySize;
        this->_errorPages = src._errorPages;
        this->_cookies = src._cookies;
        this->_modified = src._modified;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Server::Server(void) : Location(), _name(""), _maxClientBodySize(2000000), _modified(-1)
{}

Server::Server(Server const &src) : Location(src), _port(src._port), _name(src._name), _socketFd(src._socketFd),
    _maxClientBodySize(src._maxClientBodySize), _errorPages(src._errorPages), _cookies(src._cookies), _modified(src._modified)
{}

Server::~Server(void)
{}

//GETTERS ---------------------------------------------------------

std::vector<int> Server::getPorts() const
{
    return (this->_port);
}

std::string Server::getName() const
{
    return (this->_name);
}

std::vector<int> Server::getSocketFds() const
{
    return (this->_socketFd);
}

std::map<int, std::string>	Server::getErrorPages() const
{
    return (this->_errorPages);
}


long long Server::getMaxBodyClientSize() const
{
    return this->_maxClientBodySize;
}

std::vector<Cookies> &Server::getCookies()
{
    return this->_cookies;
}

int Server::getModified() const 
{
    return this->_modified;
}

//SETTERS ---------------------------------------------------------

void Server::addErrorPage(int key, std::string value)
{
    if (this->_errorPages.find(key) == this->_errorPages.end())
		this->_errorPages.insert(std::make_pair(key, value));
	else
		this->_errorPages[key] = value;
}

void Server::addPort(int port)
{
    this->_port.push_back(port);
}

void Server::setMaxBodyClientSize(long long size)
{
    this->_maxClientBodySize = size;
}

void Server::setName(std::string name)
{
    this->_name = name;
}

void Server::setModified(int index)
{
    this->_modified = index;
}

void Server::addSocketFd(int fd)
{
    this->_socketFd.push_back(fd);
}

void Server::addCookies(Cookies newCookie)
{
    this->_cookies.push_back(newCookie);
}

void Server::delCookies(std::string id)
{
    for (size_t i = 0; i < this->_cookies.size(); i++)
    {
        if (this->_cookies[i].getId() == id)
        {
            this->_cookies.erase(this->_cookies.begin() + i);
            return ;
        }
    }
}
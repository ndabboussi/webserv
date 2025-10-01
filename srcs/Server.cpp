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
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

// Server::Server(void): Location();

Server::Server(void) : Location(), _port(-1), _name(""), _socketFd(-1), _maxClientBodySize(200000)
{}

Server::Server(Server const &src) : Location(src), _port(src._port), _name(src._name), _socketFd(src._socketFd), _maxClientBodySize(src._maxClientBodySize)
{}

Server::~Server(void)
{}

//GETTERS ---------------------------------------------------------

int Server::getPort() const
{
    return (this->_port);
}

std::string Server::getName() const
{
    return (this->_name);
}

int Server::getSocketFd() const
{
    return (this->_socketFd);
}

long long Server::getMaxBodyClientSize() const
{
    return this->_maxClientBodySize;
}

//SETTERS ---------------------------------------------------------

void Server::setPort(int port)
{
    this->_port = port;
}

void Server::setMaxBodyClientSize(long long size)
{
    this->_maxClientBodySize = size;
}

void Server::setName(std::string name)
{
    this->_name = name;
}


void Server::setSocketFd(int fd)
{
    this->_socketFd = fd;
}

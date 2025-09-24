#include "Server.hpp"

//Operators----------------------------------------------------------------

Server &Server::operator=(Server const &src)
{
    if (this != &src)
    {
        Location::operator=(src);
        this->_port = src._port;
        this->_name = src._name;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

// Server::Server(void): Location();

Server::Server(void) : Location(), _port(-1), _name(""), _socketFd(-1)
{}

Server::Server(Server const &src) : Location(src), _port(src._port), _name(src._name), _socketFd(src._socketFd)
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

//SETTERS ---------------------------------------------------------

void Server::setPort(int port)
{
    this->_port = port;
}

void Server::setName(std::string name)
{
    this->_name = name;
}


void Server::setSocketFd(int fd)
{
    this->_socketFd = fd;
}

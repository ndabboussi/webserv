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

Server::Server(void) : Location(), _port(0), _name("") {}

Server::Server(Server const &src) : Location(src), _port(src._port), _name(src._name) {}

Server::~Server(void) {}

//Member functions---------------------------------------------------------


#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <sys/socket.h> // For socket functions
# include <netinet/in.h> // For sockaddr_in
# include <cstdlib>      // For exit() and EXIT_FAILURE
# include <iostream>     // For cout
# include <unistd.h>     // For read
# include <cerrno>

class Location
{
	protected:
		std::string	_path;
		std::string	_root;
		uint8_t		_methods;

	public:
		Location(/* args */);
		~Location();
};

Location::Location(/* args */)
{
}

Location::~Location()
{
}

#endif
#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <sys/socket.h> // For socket functions
# include <netinet/in.h> // For sockaddr_in
# include <cstdlib>      // For exit() and EXIT_FAILURE
# include <iostream>     // For cout
# include <unistd.h>     // For read
# include <cerrno>
# include <map>
# include <vector>

typedef enum e_methods
{
	GET = 1,
	POST,
	DELETE = 4
}	t_methods;

class Location
{
	protected:
		std::vector<Location>				_locations;
		std::map<std::string, std::string>	_data;
		bool								_alias;
		uint8_t								_methods;
		
	public:
		Location(void);
		Location(Location const &src);
		Location &operator=(Location const &src);
		~Location(void);

	public:
		std::vector<Location> const					getLocations() const;
		std::map<std::string, std::string> const	getData() const;
		bool const									getAlias() const;
		uint8_t const								getMethods() const;
		void										addLocations(Location newLoc);
		void										addData(std::string key, std::string value);
		void										setAlias(bool alias);
		void										setMethods(uint8_t methods);
};


#endif
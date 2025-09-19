#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <sys/socket.h> // For socket functions
# include <netinet/in.h> // For sockaddr_in
# include <cstdlib>      // For exit() and EXIT_FAILURE
# include <iostream>     // For cout
# include <unistd.h>     // For read
# include <string>
# include <algorithm>

# include <sstream>  // pour std::istringstream
# include <fstream>  // pour std::ifstream
# include <stdexcept> 

# include <cerrno>
# include <map>
# include <vector>

# define GREEN "\033[32m"
# define RESET "\033[0m"
# define YELLOW "\033[33m"
# define RED "\033[31m"
# define BLUE "\033[34m"
# define GRAY "\033[90m"
# define PINK "\033[35m"

typedef enum e_methods
{
	GET = 1,
	POST,
	DELETE = 4
}	t_methods;

class Location
{
	private:
		std::string _path;

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
		bool										getAlias() const;
		uint8_t										getMethods() const;
		std::string									getPath() const;
		void										addLocations(Location newLoc);
		void										addData(std::string key, std::string value);
		void										cpyData(std::map<std::string, std::string> data);
		void										setAlias(bool alias);
		void										setMethods(uint8_t methods);
		void										setPath(std::string path);
};


#endif
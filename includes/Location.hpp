#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <sys/socket.h> // For socket functions
# include <netinet/in.h> // For sockaddr_in
# include <cstdlib>      // For exit() and EXIT_FAILURE
# include <iostream>     // For cout
# include <unistd.h>     // For read
# include <string>
# include <dirent.h>	 // for opendir
# include <algorithm>

#include <cstring>      // memset
#include <arpa/inet.h>  // sockaddr_in, inet_addr

# include <sstream>  // pour std::istringstream
# include <fstream>  // pour std::ifstream
# include <stdexcept> 

# include <cerrno>
# include <map>
# include <vector>

# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RED "\033[31m"
# define BLUE "\033[34m"
# define GREY "\033[90m"
# define PINK "\033[35m"
# define RESET "\033[0m"

#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define ITALIC "\033[3m"

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
		bool		_autoIndex;

		std::vector<std::string>	_cgiPaths;
		std::vector<std::string>	_cgiExts;

	protected:
		std::vector<Location>				_locations;
		std::map<std::string, std::string>	_data;
		uint8_t								_methods;
		
	public:
		Location(void);
		Location(Location const &src);
		Location &operator=(Location const &src);
		~Location(void);

	public:
		std::vector<Location> const					getLocations() const;
		std::map<std::string, std::string> const	getData() const;
		uint8_t										getMethods() const;
		std::string									getPath() const;
		bool										getAutoIndex() const;

		void										addLocations(Location newLoc);
		void										addData(std::string key, std::string value);


		void										cpyData(std::map<std::string, std::string> data);
		void										setMethods(uint8_t methods);
		void										setPath(std::string path);
		void										setAutoIndex(bool flag);

		std::vector<std::string>					getCgiPath() const;
		std::vector<std::string>					getCgiExt() const;
		void										addCgiPath(const std::string &path);
		void										addCgiExt(const std::string &ext);
};

#endif
#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include <fstream>
# include <sstream>
# include <stdexcept>
# include <map>
# include <vector>
# include <string>

#define GREY	"\033[90m"
#define PINK	"\033[35m"
#define GREEN	"\033[32m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define	RESET	"\033[0m"


class	 Server : public Location
{
	private:
		std::map<std::string, std::string>	_data;
		// int			_port;
		// std::string	_name;
	public:
		void addLocation(const Location &loc)
		{
			_locations.push_back(loc);
		}

		const std::vector<Location> &getLocations() const
		{
			return _locations;
		}
};

void	parsing(std::vector<Server> &servers, std::string configFile);

#endif
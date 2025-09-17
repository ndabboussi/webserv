#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include <map>
# include <vector>

class	 Server : public Location
{
	private:
		std::map<std::string, std::string>	_data;
		// int			_port;
		// std::string	_name;
	public:

};

void	parsing(std::vector<Server> &servers, std::string configFile);

#endif
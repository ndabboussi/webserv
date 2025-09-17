#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include <map>

class	 Serveur : public Location
{
	private:
		std::map<std::string, std::string>	_data;
		// int			_port;
		// std::string	_name;
	public:

};

#endif
#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"

class	 Server : public Location
{
	private:
		int			_port;
		std::string	_name;

	public:

};

void	parsing(std::vector<Server> &servers, std::string configFile);

#endif
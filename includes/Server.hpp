#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"

class	 Server : public Location
{
	private:
		int			_port;
		std::string	_name;

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
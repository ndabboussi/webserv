#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"

class	 Server : public Location
{
	private:
		int			_port;
		std::string	_name;

	public:
		Server(void);
		Server(Server const &src);
		~Server(void);
		Server &operator=(Server const &src);

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
void	printTokens(const std::vector<std::string> &tokens);

#endif
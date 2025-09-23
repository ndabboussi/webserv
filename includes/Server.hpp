#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include "parsingRequest.hpp"

struct HttpRequest;

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
		int			getPort() const;
		std::string	getName() const;
		void		setPort(int port);
		void		setName(std::string name);
};

void	parsing(std::vector<Server> &servers, std::string configFile);
void	printTokens(const std::vector<std::string> &tokens);
int		launchServer(const std::vector<Server> &servers);
void	printServers(const std::vector<Server> &servers);
void	printLocation(const std::vector<Location> &locations);
void	sendResponse(int client_fd, const HttpRequest &request);

#endif
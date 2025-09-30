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
		int			_socketFd;
		long long		_maxClientBodySize;

	public:
		Server(void);
		Server(Server const &src);
		~Server(void);
		Server &operator=(Server const &src);

	public:
		int			getPort() const;
		std::string	getName() const;
		int         getSocketFd() const;
		long long		getMaxBodyClientSize() const;

		void		setPort(int port);
		void		setMaxBodyClientSize(long long size);
		void		setName(std::string name);
		void        setSocketFd(int fd);
};

void	parsing(std::vector<Server> &servers, std::string configFile);
void	printTokens(const std::vector<std::string> &tokens);
int		launchServer(std::vector<Server> &servers);
void	printServers(const std::vector<Server> &servers);
void	printLocation(const std::vector<Location> &locations);
void	sendResponse(int client_fd, const HttpRequest &request);

#endif
#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include "parsingRequest.hpp"
# include "HttpResponse.hpp"

struct	HttpRequest;
struct	HttpResponse;

class	 Server : public Location
{
	private:
		std::vector<int>			_port;
		std::string					_name;
		std::vector<int>			_socketFd;
		long long					_maxClientBodySize;
		std::map<int, std::string>	_errorPages;	


	public:
		Server(void);
		Server(Server const &src);
		~Server(void);
		Server &operator=(Server const &src);
		

	public:
		std::vector<int>			getPorts() const;
		std::vector<int>			getSocketFds() const;
		std::map<int, std::string>	getErrorPages() const;
		std::string					getName() const;
		long long					getMaxBodyClientSize() const;

		void						addPort(int port);
		void						addSocketFd(int fd);
		void						addErrorPage(int key, std::string value);
		void						setMaxBodyClientSize(long long size);
		void						setName(std::string name);
};

void	parsing(std::vector<Server> &servers, std::string configFile);
void	printTokens(const std::vector<std::string> &tokens);
int		launchServer(std::vector<Server> &servers);
void	printServers(const std::vector<Server> &servers);
void	printLocation(const std::vector<Location> &locations);
void	sendResponse(int client_fd, const HttpRequest &request, const Server &server);

#endif
#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include "parsingRequest.hpp"
# include "HttpResponse.hpp"
# include "debugUtils.hpp"
# include "Cookies.hpp"
# include <fcntl.h> // for openmain
# include <signal.h> // for signals
# include <csignal> //for SIGINT


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
		std::vector<Cookies>		_cookies;
		int							_modified;

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
		std::vector<Cookies>		&getCookies();
		int							getModified() const;

		void						addPort(int port);
		void						addSocketFd(int fd);
		void						addErrorPage(int key, std::string value);
		void						setMaxBodyClientSize(long long size);
		void						setName(std::string name);
		void						addCookies(Cookies newCookie);
		void						delCookies(std::string id);
		void						setModified(int index);
};

void		parsing(std::vector<Server> &servers, std::string configFile);
void		printTokens(const std::vector<std::string> &tokens);
int			launchServer(std::vector<Server> &servers);
void		printServers(const std::vector<Server> &servers);
void		printLocation(const std::vector<Location> &locations);
bool		handleClient(int client_fd, Server &servers, int serverPort);
void		sendResponse(int client_fd, const HttpRequest &request, Server &server);
void		handleSignal(int signum);
std::string	toString(size_t value);

#endif
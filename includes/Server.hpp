#ifndef SERVER_HPP
# define SERVER_HPP

# include "Location.hpp"
# include "Response.hpp"
# include "Cookies.hpp"
# include "PersonalInfos.hpp"
# include "Client.hpp"
# include <fcntl.h> // for openmain
# include <signal.h> // for signals
# include <csignal> //for SIGINT

class	Response;
class	PersonalInfos;
class	Client;

struct	HttpRequest;

struct Context
{
	std::vector<int> allServerFds;
	std::vector<int> allClientFds;
};

class	Server : public Location
{
	private:
		std::vector<int>						_port;
		std::string								_name;
		std::vector<int>						_socketFd;
		long long								_maxClientBodySize;
		std::map<int, std::string>				_errorPages;
		std::vector<Cookies>					_cookies;
		int										_modified;
		std::map<std::string, PersonalInfos>	_accountIdToInfos;
		std::vector<PersonalInfos>				_accounts;
		bool									_fork;

	public:
		Server(void);
		Server(Server const &src);
		~Server(void);
		Server &operator=(Server const &src);

	public:
		const std::vector<int>						&getPorts() const;
		const std::vector<int>						&getSocketFds() const;
		const std::map<int, std::string>			&getErrorPages() const;
		const std::string							&getName() const;
		long long									getMaxBodyClientSize() const;
		std::vector<Cookies>						&getCookies();
		int											getModified() const;
		const std::map<std::string, PersonalInfos>	&getAccountIdToInfos() const;
		const std::vector<PersonalInfos>			&getAccounts() const;
		int											getFork() const;

		void										addPort(int port);
		void										addSocketFd(int fd);
		void										addErrorPage(int key, std::string value);
		void										setMaxBodyClientSize(long long size);
		void										setName(std::string name);
		void										addCookies(Cookies newCookie);
		void										delCookies(std::string id);
		void										setModified(int index);
		void										addAccountIdToInfos(std::string accountId, PersonalInfos info);
		void										delAccountIdToInfos(std::string accountId);
		bool										addAccounts(PersonalInfos info);
		void										setFork(int flag);

		int											isValidUser(std::string username, std::string password) const;
};

void		parsing(std::vector<Server> &servers, std::string configFile);
int			launchServer(std::vector<Server> &servers);
void		sendResponse(int client_fd, HttpRequest &request, Server &server, Context &context);
void		handleSignal(int signum);
std::string	toString(size_t value);

void		printServers(const std::vector<Server> &servers);
void		printLocation(const std::vector<Location> &locations);
void		printTokens(const std::vector<std::string> &tokens);
void		debugPrintConfig(const std::vector<Server>& servers);
void		debugPrintRequest(const HttpRequest& req);

#endif
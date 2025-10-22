#ifndef CLIENT_HPP

# define CLIENT_HPP

# include <sys/stat.h>
# include "Server.hpp"

struct	Context;
class	Server;

struct HttpRequest
{
	int									statusCode;
	std::string 						method;
	std::string							path;
	uint8_t								methodPath;
	std::string							version;
	std::map<std::string, std::string>	header;
	std::map<std::string, std::string>	body;
	std::vector<std::string>			fileNames;
	std::string							url;
	std::string							autoIndexFile;
	std::string							jsonResponse;
	std::vector<char>					rawBody;
	bool								isCgi;
	int									serverPort;
};

class Client
{
	private:
		HttpRequest	_request;
		bool		_parsed;
		int			_clientFd;
		size_t		_indexServer;
		int			_port;

		//for request
		struct timeval	_time;
		int				_checkName;
		int				_checkMths;
		std::string		_data;//store data from request or CGI
		long long		_content_length;
		size_t			_endHeader;
		int				_firstRead;
		int				_chunked;

		//if request is chunked
		int			_firstChunk;
		int			_continue;
		std::string	_before;
		long long	_bodySize;
		long long	_left;

		//CGI handling
		pid_t		_cgiPid;
		int			_cgiOutputFd;
		bool		_cgiRunning;
		bool		_cgiToSend;
		std::string	_cgiBuffer;

	private:
		int 	checkName(Server &server);
		void	checkFirstLine();
		int		loadByChunk(const Server &server);
		void 	firstRead(Server &server);
		void 	otherRead(Server &server);
		void	sendErrorAndReturn(std::string errMsg, int error);

	public:
		Client(void);
		Client(int clientFd, size_t indexServer, int port);
		Client(Client const &src);
		Client &operator=(Client const &src);
		~Client(void);

	public:
		pid_t			getCgiPid() const;
		int				getCgiOutputFd() const;
		bool			isCgiRunning() const;
		bool			isCgiToSend() const;
		std::string		getCgiBuffer() const;
		struct timeval	getTime() const;

		int				getClientFd(void) const;
		size_t			getIndexServer(void) const;
		int				getPort(void) const;
		HttpRequest		getRequest(void) const;
		int				getParsed(void) const;

		void	setClientFd(int fd);
		void	setIndexServer(size_t index);
		void	setPort(int port);
		void	setRequest(HttpRequest req);

		void	setCgiPid(pid_t pid);
		void	setCgiOutputFd(int fd);
		void	setCgiRunning(bool flag);
		void	setCgiToSend(bool flag);
		void	setCgiBuffer(std::string buffer);
		void	setTime(timeval value);

		void	handleClientRead(Server &server);
		void	handleClientWrite(Server &server, Context &context);
		bool	checkTimeOut();

};

HttpRequest	parseHttpRequest(const std::string &rawRequest, Server &server);
int			checkDot(std::string &path, HttpRequest &req);
int			parsePath(HttpRequest &req, const Server &server);
int			parseBody(HttpRequest &req, std::istringstream &requestStream);
int			isAFile(std::string path);
int			error301(HttpRequest &req);
int			error400(HttpRequest &req);
int			error403(HttpRequest &req, std::string &tmp);
int			error404(HttpRequest &req, std::string &tmp);
int			error405(HttpRequest &req);
int			error413(HttpRequest &req);
int			error414(HttpRequest &req);
int			error500(HttpRequest &req);
int			error501(HttpRequest &req);
int			error505(HttpRequest &req);

#endif
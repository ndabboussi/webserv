#ifndef CLIENT_HPP

# define CLIENT_HPP

# include "Server.hpp"

struct Context;

class Client
{
	private:
		int			_clientFd;
		size_t		_indexServer;
		int			_port;
		int			_checkName;
		std::string	_data;//store data from request or CGI
		long long	_content_length;
		size_t		_endHeader;
		int			_firstRead;
		int			_chunked;

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
		std::string	_cgiBuffer;

	private:
		int checkName(Server &server, Context &context);
		int	loadByChunk(const Server &server);

	public:
		Client(void);
		Client(int clientFd, size_t indexServer, int port);
		Client(Client const &src);
		Client &operator=(Client const &src);
		~Client(void);

	public:
		int			getClientFd(void) const;
		size_t		getIndexServer(void) const;
		int			getPort(void) const;

		pid_t		getCgiPid() const;
		int			getCgiOutputFd() const;
		bool		getCgiRunning() const;
		std::string	getCgiBuffer() const;


		void	setClientFd(int fd);
		void	setIndexServer(size_t index);
		void	setPort(int port);

		void	setCgiPid(pid_t pid);
		void	setCgiOutputFd(int fd);
		void	setCgiRunning(bool flag);
		void	setCgiBuffer(std::string buffer);

		bool	handleClient(Server &server, Context &context);

};

#endif
#ifndef CLIENT_HPP

# define CLIENT_HPP

# include "Server.hpp"

class Client
{
	private:
		int			_clientFd;
		size_t		_indexServer;
		int			_port;
		int			_checkName;
		std::string	_data;
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

	private:
		int checkName(Server &server);
		int	loadByChunk(const Server &server);

	public:
		Client(void);
		Client(int clientFd, size_t indexServer, int port);
		Client(Client const &src);
		Client &operator=(Client const &src);
		~Client(void);

	public:
		int		getClientFd(void) const;
		size_t	getIndexServer(void) const;
		int		getPort(void) const;

		void	setClientFd(int fd);
		void	setIndexServer(size_t index);
		void	setPort(int port);

		bool	handleClient(Server &server);

};

#endif
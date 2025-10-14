#ifndef CLIENT_HPP

# define CLIENT_HPP

# include "Server.hpp"

class Client
{
	private:
		int			_clientFd;
		size_t		_indexServer;
		int			_port;
		std::string	_data;
		size_t		_content_length;
		size_t		_endHeader;
		int			_firstRead;
		int			_chunked;


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

		bool handleClient(Server &server);

};

#endif
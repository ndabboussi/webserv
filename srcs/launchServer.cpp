#include "Server.hpp"

#define SERVEPORT 8080
#define BUFSIZE 4096
#define SERVER_BACKLOG 100

bool	handleClient(int client_fd, const Server &servers)
{
	std::string	data;
	char		buffer[BUFSIZE] = {0};
	int			bytes_read;
	size_t		content_length = 0;
	bool		stored_header = false;

	while (1)
	{
		bytes_read = read(client_fd, buffer, sizeof(buffer));
		if (bytes_read < 0)
		{
			std::cerr << RED "Erreur de lecture depuis le client. errno: " << errno << RESET << std::endl;
			return (false);
		}
		if (bytes_read == 0)
			break;
		data.append(buffer, bytes_read);

		if (!stored_header)
		{
			size_t end = data.find("\r\n\r\n");
			if (end != std::string::npos)
			{
				stored_header = true;

				size_t pos = data.find("Content-Length:");
				if (pos != std::string::npos)
				{
					pos += 15;
					content_length = std::atoi(data.c_str() + pos);
				}

				if (content_length > 0)
				{
					size_t	content_start = data.find("\r\n\r\n");
					size_t	size = data.size() - (content_start + 4);

					while (size < content_length)
					{
						bytes_read = read(client_fd, buffer, sizeof(buffer));
						if (bytes_read <= 0)
							break;
						data.append(buffer, bytes_read);
						size += bytes_read;
					}
					break;
				}
				else
					break;
			}
		}
	}

	if (data.empty())
		return false;

	std::cout << PINK << data << RESET << std::endl;//logger

	HttpRequest request = parseHttpRequest(data.c_str(), servers);
	sendResponse(client_fd, request);
	return true;
}

// void acceptConnect(const Server &servers)
// {
// 	while (true)
// 	{
// 		struct sockaddr_in clientAdrr;
// 		socklen_t clientLen = sizeof(clientAdrr);

// 		int client_fd = accept(servers.getSocketFd(), (struct sockaddr *)&clientAdrr, &clientLen);
// 		if (client_fd < 0)
// 		{
// 			std::cerr << RED "Failed to grab connection. errno: " << errno << RESET << std::endl;
// 			continue;
// 		}
// 		//char client_address[INET_ADDRSTRLEN];
// 		char client_address[BUFSIZE];
// 		inet_ntop(AF_INET, &clientAdrr, client_address, BUFSIZE);//debug
// 		printf(PINK "Client connection: %s\n" RESET, client_address);//debug

// 		handleClient(client_fd, servers);
// 		close(client_fd);
// 	}
// }

void	bindAndListen(int server_fd, int port)
{
	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *)&sockAddress, sizeof(sockAddress)) < 0)
	{
		std::cerr << RED "Failed to bind to port " << port << ". errno: " << errno << RESET << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 10) < 0)
	{
		std::cerr << RED "Failed to listen on socket. errno: " << errno << RESET << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	std::cout << GREEN "Server running on http://localhost:" << port << RESET << std::endl;
}

int createServerSocket(int port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);//0 = protocol nb = TCP
	if (server_fd < 0)
	{
		std::cerr << RED "Failed to create socket. errno: " << errno << RESET << std::endl;
		return -1;
	}

	const int flag = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	{
		std::cerr << RED "Failed to set socket options. errno: " << errno << RESET << std::endl;
		close(server_fd);
		return -1;
	}

	bindAndListen(server_fd, port);
	return server_fd;
}

// ------------------------------------ LAUNCHSERVER ------------------------------------- //
int launchServer(std::vector<Server> &servers)
{
	if (servers.empty())
	{
		std::cerr << RED "No server configuration found." << RESET << std::endl;
		return 1;
	}

	for(size_t i = 0; i < servers.size(); i++)
	{
		int	port = 8080;
		if (servers[i].getPort() > 0)
			port = servers[i].getPort();
		int server_fd = createServerSocket(port);
		if (server_fd < 0)
			return 1;
		servers[i].setSocketFd(server_fd);
	}

	std::vector<int>	client_fds_vec;
	std::map<int, size_t> client_server_map;

	while (true)
	{
		fd_set	readfds;
		FD_ZERO(&readfds);
		int	max_fd = 0;

		//servers
		for (size_t	i = 0; i < servers.size(); i++)
		{
			int	fd = servers[i].getSocketFd();
			FD_SET(fd, &readfds);
			if (fd > max_fd)
				max_fd = fd;
		}

		//clients
		for (size_t i = 0 ; i < client_fds_vec.size(); i++)
		{
			FD_SET(client_fds_vec[i], &readfds);
			if (client_fds_vec[i] > max_fd)
				max_fd = client_fds_vec[i];
		}

		int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
		if (activity < 0)
		{
			std::cerr << RED "Select() error. errno: " << errno << RESET << std::endl;
			continue;
		}

		//check if incoming connexion
		for (size_t i = 0; i < servers.size(); i++)
		{
			int server_fd = servers[i].getSocketFd();
			if (FD_ISSET(server_fd, &readfds))
			{
				struct sockaddr_in clientAddr;
				socklen_t 			len = sizeof(clientAddr);
				int client_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &len);
				if (client_fd >= 0)
				{
					std::cout << UNDERLINE GREEN "[+] New client accepted on port " 
                              << servers[i].getPort() << RESET << std::endl;
					client_fds_vec.push_back(client_fd);
					client_server_map[client_fd] = i;
				}
			}
		}

		//check clients activity
		for (size_t i = 0; i < client_fds_vec.size(); i++)
		{
			int fd = client_fds_vec[i];
			if (FD_ISSET(fd, &readfds))
			{
				size_t	server_index = client_server_map[fd];
				bool keep = handleClient(fd, servers[server_index]);
				if (!keep)
				{
					close(fd);
					client_server_map.erase(fd);
					client_fds_vec.erase(client_fds_vec.begin() + i);
					std::cout << UNDERLINE GREY "[-] Client REMOVED from connections " 
                              << servers[server_index].getPort() << RESET << std::endl;
					continue;
				}
			}
			i++;
		}
	}
	for(size_t i = 0; i < servers.size(); i++)
		close(servers[i].getSocketFd());
	for(size_t i = 0; i < client_fds_vec.size(); i++)
		close(client_fds_vec[i]);
	return 0;
}

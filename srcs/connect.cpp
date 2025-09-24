#include "Server.hpp"

#define SERVEPORT 8080
#define BUFSIZE 4096
#define SERVER_BACKLOG 100

void handleClient(int client_fd, const Server &servers)
{
	char buffer[BUFSIZE] = {0};
	int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
	if (bytes_read <= 0)
	{
		close(client_fd);
		return;
	}

	buffer[bytes_read] = '\0';
	std::cout << PINK << buffer << RESET << std::endl;

	HttpRequest request = parseHttpRequest(buffer, servers);
	sendResponse(client_fd, request);
	close(client_fd);
}

void acceptConnect(const Server &servers)
{
	while (true)
	{
		struct sockaddr_in clientAdrr;
		socklen_t clientLen = sizeof(clientAdrr);

		int client_fd = accept(servers.getSocketFd(), (struct sockaddr *)&clientAdrr, &clientLen);
		if (client_fd < 0)
		{
			std::cerr << RED "Failed to grab connection. errno: " << errno << RESET << std::endl;
			continue;
		}
		//char client_address[INET_ADDRSTRLEN];
		char client_address[BUFSIZE];
		inet_ntop(AF_INET, &clientAdrr, client_address, BUFSIZE);//debug
		printf(PINK "Client connection: %s\n" RESET, client_address);//debug

		handleClient(client_fd, servers);
		close(client_fd);
	}
}

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
		{
			port = servers[i].getPort();
		}
		int server_fd = createServerSocket(port);
		if (server_fd < 0)
			return 1;
		servers[i].setSocketFd(server_fd);
	}

	acceptConnect(servers[0]);
	// while (true)
	// {
	// 	struct sockaddr_in clientAdrr;
	// 	socklen_t clientLen = sizeof(clientAdrr);

	// 	int client_fd = accept(servers[0].getSocketFd(), (struct sockaddr *)&clientAdrr, &clientLen);
	// 	if (client_fd < 0)
	// 	{
	// 		std::cerr << RED "Failed to grab connection. errno: " << errno << RESET << std::endl;
	// 		continue;
	// 	}
	// 	//char client_address[INET_ADDRSTRLEN];
	// 	char client_address[BUFSIZE];
	// 	inet_ntop(AF_INET, &clientAdrr, client_address, BUFSIZE);//debug
	// 	printf(PINK "Client connection: %s\n" RESET, client_address);//debug

	// 	handleClient(client_fd, servers[0]);
	// 	close(client_fd);
	// }
	for(size_t i = 0; i < servers.size(); i++)
		close(servers[i].getSocketFd());
	return 0;
}


// struct ServerSocket {
//     int fd;
//     int port;
//     Server *server;
// };
// std::vector<ServerSocket> sockets;


// fd_set readfds;
// int max_fd = -1;
// while (true) {
//     FD_ZERO(&readfds);
//     for (size_t i = 0; i < sockets.size(); ++i) {
//         FD_SET(sockets[i].fd, &readfds);
//         if (sockets[i].fd > max_fd) max_fd = sockets[i].fd;
//     }

//     int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
//     if (activity < 0) continue; // erreur ou signal

//     for (size_t i = 0; i < sockets.size(); ++i) {
//         if (FD_ISSET(sockets[i].fd, &readfds)) {
//             int client_fd = accept(sockets[i].fd, ...);
//             // Ici tu sais sur quel serveur/port le client s'est connecté :
//             handleClient(client_fd, *sockets[i].server);
//         }
//     }
// }



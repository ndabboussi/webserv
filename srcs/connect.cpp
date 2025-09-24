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

	// acceptConnect(servers[0]);
	while (true)
	{
		struct sockaddr_in clientAdrr;
		socklen_t clientLen = sizeof(clientAdrr);

		int client_fd = accept(servers[0].getSocketFd(), (struct sockaddr *)&clientAdrr, &clientLen);
		if (client_fd < 0)
		{
			std::cerr << RED "Failed to grab connection. errno: " << errno << RESET << std::endl;
			continue;
		}
		//char client_address[INET_ADDRSTRLEN];
		char client_address[BUFSIZE];
		inet_ntop(AF_INET, &clientAdrr, client_address, BUFSIZE);//debug
		printf(PINK "Client connection: %s\n" RESET, client_address);//debug

		handleClient(client_fd, servers[0]);
		close(client_fd);
	}
	for(size_t i = 0; i < servers.size(); i++)
		close(servers[i].getSocketFd());
	return 0;
}


fd_set readfds;
int max_fd = -1;
while (true) {
    FD_ZERO(&readfds);
    for (size_t i = 0; i < sockets.size(); ++i) {
        FD_SET(sockets[i].fd, &readfds);
        if (sockets[i].fd > max_fd) max_fd = sockets[i].fd;
    }

    int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0) continue; // erreur ou signal

    for (size_t i = 0; i < sockets.size(); ++i) {
        if (FD_ISSET(sockets[i].fd, &readfds)) {
            int client_fd = accept(sockets[i].fd, ...);
            // Ici tu sais sur quel serveur/port le client s'est connecté :
            handleClient(client_fd, *sockets[i].server);
        }
    }
}


std::vector<int> client_fds;  // stocke tous les clients connectés

while (true)
{
    fd_set readfds;
    // FD_ZERO(&readfds);

    int max_fd = 0;

    // 1. Ajouter tous les serveurs
    for (size_t i = 0; i < servers.size(); ++i)
    {
        int fd = servers[i].getSocketFd();
        FD_SET(fd, &readfds);
        if (fd > max_fd) max_fd = fd;
    }

    // 2. Ajouter tous les clients
    for (size_t i = 0; i < client_fds.size(); ++i)
    {
        FD_SET(client_fds[i], &readfds);
        if (client_fds[i] > max_fd) max_fd = client_fds[i];
    }

    // 3. Appeler select()
    int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (ready < 0)
    {
        perror("select");
        break;
    }

    // 4. Vérifier les serveurs
    for (size_t i = 0; i < servers.size(); ++i)
    {
        int server_fd = servers[i].getSocketFd();
        if (FD_ISSET(server_fd, &readfds))
        {
            // Nouveau client
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd >= 0)
                client_fds.push_back(client_fd);
        }
    }

    // 5. Vérifier les clients
    for (size_t i = 0; i < client_fds.size(); )
    {
        int fd = client_fds[i];
        if (FD_ISSET(fd, &readfds))
        {
            if (!handleClient(fd, /*server*/)) 
            {
                close(fd);
                client_fds.erase(client_fds.begin() + i);
                continue; // ne pas incrementer i
            }
        }
        ++i;
    }
}



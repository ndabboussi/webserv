#include "Server.hpp"

#define SERVERPORT 8080
#define SERVER_BACKLOG 100

extern volatile bool serverRunning;

//------------------------------------ SOCKETS AND BIND -------------------------------------//

/* FUNCTION: bindAndListen
 * -----------------------------------------------------
 * Binds a socket to a given port and starts listening 
 * for incoming TCP connections.
 * Parameters:
 *  - server_fd: The socket file descriptor
 *  - port: The port number to bind to
 * Steps:
 *  1. Create and configure a sockaddr_in struct
 *  2. Bind the socket to the specified port
 *  3. Start listening for incoming connections
 *  4. If any step fails, print an error and exit */
int	bindAndListen(int server_fd, int port)
{
	struct sockaddr_in sockAddress; // Structure describing the socket’s address (IPv4)
	sockAddress.sin_family = AF_INET; // IPv4 address family
	sockAddress.sin_addr.s_addr = INADDR_ANY; // Accept connections on any local IP
	sockAddress.sin_port = htons(port); // Convert port number to network byte order

	// Try binding the socket to the specified port
	if (bind(server_fd, (struct sockaddr *)&sockAddress, sizeof(sockAddress)) < 0)
  {
		std::cerr << RED "Failed to bind to port " << port << ". errno: " << errno << RESET << std::endl;
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, SERVER_BACKLOG) < 0)	// Start listening for incoming connections
  {
		std::cerr << RED "Failed to listen on socket. errno: " << errno << RESET << std::endl;
		close(server_fd);
		return -1;
	}

	return 0;
}


/* FUNCTION: createServerSocket
 * -------------------------------------------------------
 * Creates a TCP socket, sets basic options (SO_REUSEADDR), binds it to the given port, and starts listening
 * 
 * Parameters:
 *  - port: The port to listen on.
 *
 * Returns:
 *  - A valid server socket file descriptor, or -1 on error
 *
 * Notes:
 *  - SO_REUSEADDR allows quick restart of the server after crash/restart*/

int createServerSocket(int port)
{
	// Create a TCP socket (AF_INET = IPv4, SOCK_STREAM = TCP, 0 = protocol nb = TCP)
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << RED "Failed to create socket. errno: " << errno << RESET << std::endl;
		return -1;
	}

	// Allow reusing address if the socket is in TIME_WAIT state
	const int flag = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	{
		std::cerr << RED "Failed to set socket options. errno: " << errno << RESET << std::endl;
		close(server_fd);
		return -1;
	}

	// Bind the socket and start listening
	if (bindAndListen(server_fd, port))
		return -1;
	return server_fd;
}

void	initServersSockets(std::vector<Server> &servers, Context &context)
{
	for(size_t i = 0; i < servers.size(); i++)
	{
		std::vector<int> ports = servers[i].getPorts();
		for (size_t j = 0; j < ports.size(); j++)
		{
			int	port = 8080;
			if (ports[j] > 0)
				port = ports[j];
			int server_fd = createServerSocket(port);
			if (server_fd >= 0)
			{
				servers[i].addSocketFd(server_fd);
				context.allServerFds.push_back(server_fd);
				std::string name = (servers[i].getName().empty()) ? "localhost" : servers[i].getName();
				std::cout << GREEN "Server " << i << " running on http://" << name << ':' << port << RESET << std::endl;
			}
		}
	}
}


// Helper function to remove servers with no valid sockets
void removeEmptyServers(std::vector<Server> &servers)
{
	for (int i = static_cast<int>(servers.size()) - 1; i >= 0; i--)
	{
		std::vector<int> fds = servers[i].getSocketFds();
		if (fds.empty())
			servers.erase(servers.begin() + i);
	}
	
}

int monitorSockets(fd_set &readfds, fd_set &writefds, std::vector<Server> &servers, std::vector<Client> &clients)
{
	FD_ZERO(&readfds); // Reset all bits
	FD_ZERO(&writefds);
	int	maxFd = 0; // Track highest FD for select()

	// Register all server sockets --> handle multiple servers sockets
	for (size_t	i = 0; i < servers.size(); i++)
	{
		std::vector<int> serverFds = servers[i].getSocketFds();
		for (size_t j = 0; j < serverFds.size(); j++)
		{
			int	fd = serverFds[j];
			FD_SET(fd, &readfds);// Add to monitored set
			if (fd > maxFd)
				maxFd = fd;
		}
	}

	// Register all client sockets
	for (size_t i = 0 ; i < clients.size(); i++)
	{
		int clientFd = clients[i].getClientFd();
		if (!clients[i].getParsed())
			FD_SET(clientFd, &readfds);
		else
			FD_SET(clientFd, &writefds);

		if (clients[i].isCgiRunning())
		{
			int cgiFd = clients[i].getCgiOutputFd();
			if (cgiFd > 0)
			{
				FD_SET(cgiFd, &readfds);
				if (cgiFd > maxFd)
					maxFd = cgiFd;
			}
		}

		if (clientFd > maxFd)
			maxFd = clientFd;
	}
	return maxFd;
}

void acceptClientsConnections(fd_set &readfds, const std::vector<Server> &servers, std::vector<Client> &clients, Context &context)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		const std::vector<int> socketsFds = servers[i].getSocketFds();
		const std::vector<int> serverPorts = servers[i].getPorts();
		
		for (size_t j = 0; j < socketsFds.size(); j++)
		{
			int	server_fd = socketsFds[j];
			if (FD_ISSET(server_fd, &readfds))// If the server_fd is ready → incoming connection
			{
				struct sockaddr_in clientAddr;
				socklen_t 			len = sizeof(clientAddr);
				int client_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &len);
				if (client_fd < 0)
				{
					std::cerr << RED "Error: Accept() failure in launchSerevr()" RESET << std::endl;
					continue;
				}
				struct timeval tv;// Optional: Set socket timeout (prevents infinite read block)
				tv.tv_sec = 5; // timeout in seconds
				tv.tv_usec = 0;
				if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
					perror("setsockopt SO_RCVTIMEO");
				if (setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
					perror("setsockopt SO_SNDTIMEO");

				std::cout << UNDERLINE GREEN "[+] New client accepted on port " 
								<< serverPorts[j] << RESET << std::endl;

				clients.push_back(Client(client_fd, i, serverPorts[j]));
			}
		}
	}

	context.allClientFds.clear();//actualise Context clientFds to close in case of CGI
	context.allOutputFds.clear();
	for (size_t i = 0; i < clients.size(); i++)
	{
		context.allClientFds.push_back(clients[i].getClientFd());
		if (clients[i].getCgiOutputFd() > 0)
			context.allOutputFds.push_back(clients[i].getCgiOutputFd());
	}
}

int	handleClientsActivity(fd_set &readfds, fd_set &writefds, std::vector<Server> &servers, std::vector<Client> &clients, Context &context)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		int fd = clients[i].getClientFd();
		size_t	server_index = clients[i].getIndexServer();
		if (!clients[i].getParsed())
			clients[i].checkTimeOut();
		if (!clients[i].getParsed() && FD_ISSET(fd, &readfds))
			clients[i].handleClientRead(servers[server_index]);
		else if (clients[i].getParsed() && FD_ISSET(fd, &writefds))
		{
			clients[i].handleClientWrite(servers[server_index], context);
			if (servers[server_index].getFork())
				return 1;
			if (clients[i].getRequest().statusCode == 100
				|| (clients[i].getRequest().isCgi && !(!clients[i].isCgiRunning() && clients[i].isCgiToSend())))
				continue;
			close(fd);
			clients.erase(clients.begin() + i);
			std::cout << UNDERLINE GREY "[-] Client REMOVED from connections " 
						<< RESET << std::endl;
			i--;
		}
	}
	return 0;
}

void cleanup(std::vector<Server> &servers, std::vector<Client> &clients)
{
	for(size_t i = 0; i < servers.size(); i++)
	{
		const std::vector<int> fds = servers[i].getSocketFds();
		for (size_t j = 0; j < fds.size(); j++)
			close(fds[j]);
	}
	for(size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getCgiOutputFd() > -1)
			close(clients[i].getCgiOutputFd());
		close(clients[i].getClientFd());
	}
	std::cout << "\033[1;32m[✓] Server shutdown complete.\033[0m\n";
}

//------------------------------------ LAUNCHSERVER -------------------------------------//

/* FUNCTION: launchServer
 * -------------------------------------------------------
 * Main event loop of the server.
 * It:
 *  - Creates and binds sockets for all configured servers
 *  - Uses select() to monitor multiple sockets and clients
 *  - Accepts new client connections
 *  - Delegates client handling to handleClient()
 *
 * Parameters:
 *  - servers: a vector of configured `Server` instances
 *
 * Behavior:
 *  - Infinite loop waiting for socket activity
 *  - Uses select() to multiplex server sockets and clients
 *  - Cleans up disconnected clients automatically */

int launchServer(std::vector<Server> &servers)
{
	if (servers.empty())
	{
		std::cerr << RED "No server configuration found." << RESET << std::endl;
		return 1;
	}

	Context	context;
	// STEP 1: Create a listening socket for each port of each server

	initServersSockets(servers, context);

	removeEmptyServers(servers);

	if (servers.empty())
		return 1;

	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		std::map<std::string, std::string> map = it->getData();
		if (map.find("root") != map.end())
			restoreLocations(*it, map.find("root")->second);
	}
	
	int breake = 0;

	// STEP 2: Main event loop using select()
	std::vector<Client> clients;//structures to track clients 
	while (true && serverRunning)
	{
		fd_set	readfds; // File descriptor set for select()
		fd_set	writefds;
		int	maxFd = 0; // Track highest FD for select()
		maxFd = monitorSockets(readfds, writefds, servers, clients);
	
		struct timeval tv;
		tv.tv_sec = 0; // 0 seconds
		tv.tv_usec = 50000; //50 milliseconds (adjustable tick)
		int activity = select(maxFd + 1, &readfds, &writefds, NULL, &tv);
		if (activity < 0) // Wait for socket activity (non-blocking)
		{
			std::cerr << RED "Select() error. errno: " << errno << RESET << std::endl;
			continue;
		}

		// STEP 3: Accept new clients on correct port
		acceptClientsConnections(readfds, servers, clients, context);

		// STEP 4: Handle client activity
		breake = handleClientsActivity(readfds, writefds, servers, clients, context);
		if (breake)
			break ;
	}

	// STEP 5: Cleanup on shutdown (reached in case of CTRL+C or CGI children)
	cleanup(servers, clients);

	if (breake)
		return 1;

	return 0;
}

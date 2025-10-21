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

	for(size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i].getSocketFds().empty())
		{
			servers.erase(servers.begin() + i);
			i--;
		}
	}

	if (servers.empty())
		return 1;

	// STEP 2: Prepare structures to track clients
	std::vector<Client> clients;

	 // STEP 3: Main select() event loop
	while (true && serverRunning)
	{
		fd_set	readfds; // File descriptor set for select()
		FD_ZERO(&readfds); // Reset all bits
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
			FD_SET(clientFd, &readfds);
			if (clientFd > maxFd)
				maxFd = clientFd;
		}
	
		struct timeval tv;
		tv.tv_sec = 0; // 0 seconds
		tv.tv_usec = 50000; //50 milliseconds (adjustable tick)
		int activity = select(maxFd + 1, &readfds, NULL, NULL, &tv);
		if (activity < 0) // Wait for socket activity (non-blocking)
		{
			std::cerr << RED "Select() error. errno: " << errno << RESET << std::endl;
			continue;
		}

		// STEP 4: Check if new client is connecting to a server, accept client on the correct port
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
					// Optional: Set socket timeout (prevents infinite read block)
					struct timeval tv;
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

		context.allClientFds.clear();
		for (size_t i = 0; i < clients.size(); i++)
			context.allClientFds.push_back(clients[i].getClientFd());

		int breake = 0;
		// STEP 5: Handle activity from connected clients
		for (size_t i = 0; i < clients.size(); i++)
		{
			int fd = clients[i].getClientFd();
			if (FD_ISSET(fd, &readfds))
			{
				size_t	server_index = clients[i].getIndexServer();
				// Delegate to the HTTP handling logic
				bool keep = clients[i].handleClient(servers[server_index], context);
				if (keep == false) // If client disconnected or done → cleanup
				{
					close(fd);
					clients.erase(clients.begin() + i);
					std::cout << UNDERLINE GREY "[-] Client REMOVED from connections " 
							 << RESET << std::endl;
					i--;
					if (servers[server_index].getFork())
					{
						breake = 1;
						break;
					}
					continue;
				}
			}
		}
		if (breake)
			break ;
	}
	// STEP 6: Cleanup on shutdown (reached in case of CTRL+C or CGI children)
	for(size_t i = 0; i < servers.size(); i++)
	{
		const std::vector<int> fds = servers[i].getSocketFds();
		for (size_t j = 0; j < fds.size(); j++)
			close(fds[j]);
	}
	for(size_t i = 0; i < clients.size(); i++)
		close(clients[i].getClientFd());
	std::cout << "\033[1;32m[✓] Server shutdown complete.\033[0m\n";
	return 0;
}

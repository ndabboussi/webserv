#include <cstring>      // memset
#include <cstdlib>      // exit
#include <unistd.h>     // close
#include <arpa/inet.h>  // sockaddr_in, inet_addr
#include <sys/socket.h> // socket, bind, listen, accept
#include "Server.hpp"

int launchServer()
{
	// 1. Create a socket
	int server_fd;
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		//perror("socket");
		std::cerr << "Failed to create socket. errno: " << errno << std::endl;
		exit(EXIT_FAILURE);
	}

	const int	flag = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	{
		std::cerr << "Error. errno: " << errno << std::endl;
		return 1;
	}

	// 2. Listen to port 8080 (0.0.0.0:8080)
	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY; // toutes les interfaces
	sockAddress.sin_port = htons(8080);
	
	// 3. Link socket to the address
	if (bind(server_fd, (struct sockaddr *)&sockAddress, sizeof(sockAddress)) < 0)
	{
		//perror("bind");
		std::cerr << "Failed to bind to port 8080. errno: " << errno << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// 4. Start listening. Hold at most 10 connections in the queue
	if (listen(server_fd, 10) < 0)
	{
		//perror("listen");
		std::cerr << "Failed to listen on socket. errno: " << errno << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	std::cout << "Server running on http://localhost:8080" << std::endl;

	// 5. Grab a connection from the queue
	socklen_t sockAddrLen = sizeof(sockAddress);
	std::memset(&sockAddress, 0, sizeof(sockAddress));
	int	client_fd;
	client_fd = accept(server_fd, (struct sockaddr *)&sockAddress, &sockAddrLen);
	if (client_fd < 0)
	{
		//perror("accept");
		std::cerr << "Failed to grab connection. errno: " << errno << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// 6. Read from the connection
	char buffer[1024];
	int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
	if (bytes_read > 0)
	{
		buffer[bytes_read] = '\0';
		std::cout << "The message was: \n" << buffer << std::endl;
	}

	// 7. Send a message to the connection
	 std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 25\r\n\r\nHello from webserv!";

	send(client_fd, response.c_str(), strlen(response.c_str()), 0);

	// Close the connections
	close(client_fd);
	close(server_fd);

	return 0;
}

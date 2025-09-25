#include <cstring>      // memset
#include <cstdlib>      // exit
#include <unistd.h>     // close
#include <arpa/inet.h>  // sockaddr_in, inet_addr
#include <sys/socket.h> // socket, bind, listen, accept
#include "Server.hpp"

int main()
{
    // Create a socket (IPv4, TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Listen to port 9999 on any address
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(9999); // convert to network byte order

    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start listening. Hold at most 10 connections in the queue
    if (listen(sockfd, 10) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Grab a connection from the queue
    socklen_t addrlen = sizeof(sockaddr);
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, &addrlen);
    if (connection < 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read from the connection

    // Send a message to the connection
    while (true)
    {
        char buffer[100];
        ssize_t bytesRead = read(connection, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
        buffer[bytesRead] = '\0'; // Null-terminate to safely print
        std::cout << "The message was: " << buffer;
        }

        std::string response = "Good talking to you\n";
        send(connection, response.c_str(), response.size(), 0);
        // if (!buffer)
        //   break;
    }

    // Close the connections
    close(connection);
    close(sockfd);
}


// #include "parsingRequest.hpp"
#include "Server.hpp"

int launchServer(const std::vector<Server> &servers)
{
	// 1. Create a socket
	int server_fd = -1;
	server_fd = socket(AF_INET, SOCK_STREAM, 0);//0 = protocol nb = TCP
	if (server_fd < 0)
	{
		//perror("socket");
		std::cerr << RED "Failed to create socket. errno: " << errno << RESET << std::endl;
		exit(EXIT_FAILURE);
	}

	const int	flag = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0)
	{
		std::cerr << RED "Error. errno: " << errno << RESET << std::endl;
		close(server_fd);
		return 1;
	}

	//	1.1. Determine which port to use
	int	port = 8080;
	if (!servers.empty())
	{
		if (servers[0].getPort() > 0)
		{
			port = servers[0].getPort();
		}
	}

	// 2. Listen to port 8080 (0.0.0.0:8080)
	struct sockaddr_in sockAddress;
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = INADDR_ANY;
	sockAddress.sin_port = htons(port);//converts a nb to the network standard bytes order
	/* htonl converts a long integer (e.g. address) to a network representation */
	/* htons converts a short integer (e.g. port) to a network representation */ 
	
	// 3. Link socket to the address
	if (bind(server_fd, (struct sockaddr *)&sockAddress, sizeof(sockAddress)) < 0)
	{
		//perror("bind");
		std::cerr << RED "Failed to bind to port 8080. errno: " << errno << RESET << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// 4. Start listening. Hold at most 10 connections in the queue
	if (listen(server_fd, 10) < 0)
	{
		//perror("listen");
		std::cerr << RED "Failed to listen on socket. errno: " << errno << RESET << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	std::cout << GREEN "Server running on http://localhost:" << port << RESET <<  std::endl;

	// 5. Grab a connection from the queue
	while (true)
	{
		struct sockaddr_in clientAdrr;
		socklen_t	clientLen = sizeof(clientAdrr);
		int client_fd = accept(server_fd, (struct sockaddr *)&clientAdrr, &clientLen);
		if (client_fd < 0)
		{
			//perror("accept");
			std::cerr << RED "Failed to grab connection. errno: " << errno << RESET << std::endl;
			close(server_fd);
			continue;
		}

		char client_adress[4096];
		inet_ntop(AF_INET, &clientAdrr, client_adress, 4096);
		printf(PINK "Client connection: %s\n" RESET, client_adress);


		// 6. Read from the connection
		char buffer[4096] = {0};
		int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
		if (bytes_read <= 0)
		{
			close(client_fd);
			continue;
		}
		buffer[bytes_read] = '\0';
		std::cout << PINK <<  buffer << RESET << std::endl;
		HttpRequest request = parseHttpRequest(buffer, servers[0]);
		sendResponse(client_fd, request);
		close(client_fd);

	}
	close(server_fd);

	return 0;
}


//  #define MAX_EVENTS 10
//            struct epoll_event ev, events[MAX_EVENTS];
//            int listen_sock, conn_sock, nfds, epollfd;

//            /* Code to set up listening socket, 'listen_sock',
//               (socket(), bind(), listen()) omitted */

//            epollfd = epoll_create1(0);
//            if (epollfd == -1) {
//                perror("epoll_create1");
//                exit(EXIT_FAILURE);
//            }

//            ev.events = EPOLLIN;
//            ev.data.fd = listen_sock;
//            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
//                perror("epoll_ctl: listen_sock");
//                exit(EXIT_FAILURE);
//            }

//            for (;;) {
//                nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
//                if (nfds == -1) {
//                    perror("epoll_wait");
//                    exit(EXIT_FAILURE);
//                }

//                for (n = 0; n < nfds; ++n) {
//                    if (events[n].data.fd == listen_sock) {
//                        conn_sock = accept(listen_sock,
//                                           (struct sockaddr *) &addr, &addrlen);
//                        if (conn_sock == -1) {
//                            perror("accept");
//                            exit(EXIT_FAILURE);
//                        }
//                        setnonblocking(conn_sock);
//                        ev.events = EPOLLIN | EPOLLET;
//                        ev.data.fd = conn_sock;
//                        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
//                                    &ev) == -1) {
//                            perror("epoll_ctl: conn_sock");
//                            exit(EXIT_FAILURE);
//                        }
//                    } else {
//                        do_use_fd(events[n].data.fd);
//                    }
//                }
//            }
#include "parsingRequest.hpp"

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
		HttpRequest request = parseHttpRequest(buffer, servers[0]);

		if (!request.path.empty())
		{
			std::string filePath = request.path;
			std::ifstream file(filePath.c_str(), std::ios::binary);
			if (file)
			{
				std::vector<char> fileContent((std::istreambuf_iterator<char>(file)),
												std::istreambuf_iterator<char>());
				std::ostringstream response;
				response << "HTTP/1.1 200 OK\r\n";
				response << "Content-Type: image/jpeg\r\n";
				response << "Content-Length: " << fileContent.size() << "\r\n";
				response << "Connection: close\r\n\r\n";

				std::string headers = response.str();
				send(client_fd, headers.c_str(), headers.size(), 0);
				send(client_fd, fileContent.data(), fileContent.size(), 0);

				std::cout << GREEN "[<] Sent image: herbe.jpg (" << fileContent.size() << " bytes)" << RESET << std::endl;
			}
			else
			{
				std::string notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
				send(client_fd, notFound.c_str(), notFound.size(), 0);
			}
		}
		else
		{
			// 7. Send a message to the connection
			std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 19\r\n\r\nHello from webserv!";
			std::cout << GREEN << "[<] Response sent to client." << RESET << std::endl;
			send(client_fd, response.c_str(), strlen(response.c_str()), 0);
		}

		close(client_fd);

	}
	close(server_fd);

	return 0;
}

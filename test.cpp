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

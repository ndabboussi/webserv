#include "Server.hpp"

#define SERVEPORT 8080
#define BUFSIZE 4096
#define SERVER_BACKLOG 100

//------------------------------------ CHUNKS -------------------------------------//

static int readLine(int &client_fd, std::string &line)
{
	int byteRead = 0;
	char c = 0;

	line.clear();
	while (c != '\n')
	{
		int res = read(client_fd, &c, 1);
		if (res < 0)
			return -1;
		else if (!res)
			return byteRead;
		byteRead += res;
		line += c;
	}
	return byteRead;
}

static void deleteChunkSize(std::string data, std::string &src)
{
	std::istringstream  before(data);
	std::string			buffer;
	long long			size;
	std::vector<char>	buff;
	char				crlf[2];
	size_t pos = src.find("\r\n\r\n") + 4;
	
	while (std::getline(before, buffer))
	{
		if (!buffer.empty())
		{
			std::stringstream ss;
			ss << std::hex << buffer;
			ss >> size;
			buff.resize(size);
			src.erase(pos, buffer.size() + 1);
			if (size == 0)
				break ;
			pos += size;
			before.read(buff.data(), size);
			before.read(crlf, 2);
			if (crlf[0] != '\r' && crlf[1] != '\n')
				break ;//error bad request
			src.erase(pos, 2);
		}
	}
}

static int	loadByChunk(std::string &data, std::string afterHeader, const Server &server, int &client_fd)
{
	std::istringstream  before(afterHeader);
	long long			bodySize = 0;
	std::string			buffer;
	std::vector<char>	buff;
	long long			ByteRead;
	long long			size;
	long long			left = 0;
	
	while (std::getline(before, buffer))
	{
		if (!buffer.empty() && buffer[buffer.size() - 1] == '\r')
			buffer.erase(buffer.size() - 1);
		if (!buffer.empty())
		{
			std::stringstream ss;
			ss << std::hex << buffer;
			ss >> size;
			buff.resize(size + 2);
			before.read(buff.data(), size + 2);
			bodySize += before.gcount();
			if (size + 2 > before.gcount())
				left = size + 2 - before.gcount();
		}
	}
	if (bodySize > server.getMaxBodyClientSize())
	{
		std::cerr << RED "Error 413: Entity Too Large" << RESET << std::endl;
		return 413;
	}
	if (left > 0)
	{
		buff.resize(left + 2);
		ByteRead = read(client_fd, buff.data(), left + 2);
		if (ByteRead < 0)
			return -1;
		data.append(buff.data(), ByteRead);
	}
	while ((left = readLine(client_fd, buffer)) > 0)
	{
		data.append(buffer, 0, buffer.size());
		std::stringstream ss;
		ss << std::hex << buffer;
		ss >> size;
		if (size == 0)
			break ;
		buff.resize(size + 2);
		ByteRead = read(client_fd, buff.data(), size + 2);
		if (ByteRead < 0)
			return -1;
		bodySize += ByteRead;
		data.append(buff.data(), ByteRead);
		if (bodySize > server.getMaxBodyClientSize())
		{
			std::cerr << RED "Error 413: Entity Too Large" << RESET << std::endl;
			return 413;
		}
	}
	size_t pos = data.find("\r\n\r\n") + 4;
	deleteChunkSize(data.substr(pos, data.size() - pos), data);
	return 0;
}

//------------------------------------ HANDLE CLIENTS -------------------------------------//

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
			std::cerr << RED "Erreur de lecture depuis le client." << RESET << std::endl;// error 500 ?
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

				size_t pos;
				if ((pos = data.find("Content-Length:")) != std::string::npos)
				{
					pos += 15;
					content_length = std::atoi(data.c_str() + pos);
				}
				else if (data.find("Transfer-Encoding: chunked") != std::string::npos)
				{
					int res = loadByChunk(data, data.substr(end + 4, data.size() - end + 4), servers, client_fd);
					if (res)
					{
						HttpRequest req;
						req.error = res;
						sendResponse(client_fd, req);
						return true;
					}
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
				}
				break;
			}
		}
	}

	if (data.empty())
		return false;

	std::cout << PINK << data << RESET << std::endl;//logger

	HttpRequest request = parseHttpRequest(data, servers);
	sendResponse(client_fd, request);
	return true;
}

//------------------------------------ SOCKETS AND BIND -------------------------------------//

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

//------------------------------------ LAUNCHSERVER -------------------------------------//

void	createServersSockets(std::vector<Server> &servers)
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
			// if (server_fd < 0)
			// 	//return 1; throw exception 
			servers[i].addSocketFd(server_fd);
		}
	}
}

void	setServersSockets(std::vector<Server> &servers, int maxFd, fd_set &readfds)
{
		for (size_t	i = 0; i < servers.size(); i++)
	{
		std::vector<int> serverFds = servers[i].getSocketFds();
		for (size_t j = 0; j < serverFds.size(); j++)
		{
			int	fd = serverFds[j];
			FD_SET(fd, &readfds);
			if (fd > maxFd)
				maxFd = fd;
		}
	}
}

int launchServer(std::vector<Server> &servers)
{
	if (servers.empty())
	{
		std::cerr << RED "No server configuration found." << RESET << std::endl;
		return 1;
	}

	//createServersSockets(servers);
	//create one socket per each servers's port
	for(size_t i = 0; i < servers.size(); i++)
	{
		std::vector<int> ports = servers[i].getPorts();
		for (size_t j = 0; j < ports.size(); j++)
		{
			int	port = 8080;
			if (ports[j] > 0)
				port = ports[j];
			int server_fd = createServerSocket(port);
			if (server_fd < 0)
				return 1;
			servers[i].addSocketFd(server_fd);
		}
	}

	std::vector<int>	client_fds_vec;
	std::map<int, size_t> client_server_map;

	while (true)
	{
		//int cpt = 0;
		fd_set	readfds;
		FD_ZERO(&readfds);
		int	maxFd = 0;

		//std::cout << UNDERLINE "HERE 1: " << cpt << RESET << std::endl;
		//setServersSockets(servers, maxFd, readfds);
		//servers --> handle multiple servers sockets
		for (size_t	i = 0; i < servers.size(); i++)
		{
			std::vector<int> serverFds = servers[i].getSocketFds();
			for (size_t j = 0; j < serverFds.size(); j++)
			{
				int	fd = serverFds[j];
				FD_SET(fd, &readfds);
				if (fd > maxFd)
					maxFd = fd;
			}
		}
		//std::cout << UNDERLINE "HERE 2: " << cpt << RESET << std::endl;

		//clients
		for (size_t i = 0 ; i < client_fds_vec.size(); i++)
		{
			FD_SET(client_fds_vec[i], &readfds);
			if (client_fds_vec[i] > maxFd)
				maxFd = client_fds_vec[i];
		}
		//std::cout << UNDERLINE "HERE 3: " << cpt << RESET << std::endl;
		//timeval tv = {0, 0};
		//int activity = select(maxFd + 1, &readfds, NULL, NULL, &tv);
		int activity = select(maxFd + 1, &readfds, NULL, NULL, NULL);
		if (activity < 0)
		{
			std::cerr << RED "Select() error. errno: " << errno << RESET << std::endl;
			continue;
		}

		//std::cout << UNDERLINE "HERE 4: " << cpt << RESET << std::endl;
		//check if incoming connexion, accept client on the correct port
		for (size_t i = 0; i < servers.size(); i++)
		{
			const std::vector<int> socketsFds = servers[i].getSocketFds();
			const std::vector<int> serverPorts = servers[i].getPorts();

			for (size_t j = 0; j < socketsFds.size(); j++)
			{
				int	server_fd = socketsFds[j];
				if (FD_ISSET(server_fd, &readfds))
				{
					struct sockaddr_in clientAddr;
					socklen_t 			len = sizeof(clientAddr);
					int client_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &len);
					if (client_fd >= 0)
					{
						std::cout << UNDERLINE GREEN "[+] New client accepted on port " 
									<< serverPorts[j] << RESET << std::endl;
						client_fds_vec.push_back(client_fd);
						client_server_map[client_fd] = i;
					}
				}
			}
		}

		//std::cout << UNDERLINE "HERE 5: " << cpt << RESET << std::endl;
		//check clients activity
		for (size_t i = 0; i < client_fds_vec.size(); i++)
		{
			int fd = client_fds_vec[i];
			if (FD_ISSET(fd, &readfds))
			{
				size_t	server_index = client_server_map[fd];
				bool keep = handleClient(fd, servers[server_index]);
				//bool keep = 0;
				if (!keep)
				{
					close(fd);
					client_server_map.erase(fd);
					client_fds_vec.erase(client_fds_vec.begin() + i);
					// std::cout << UNDERLINE GREY "[-] Client REMOVED from connections " 
					// 		<< servers[server_index].getPorts() << RESET << std::endl;
					i--;
					continue;
				}
			}
			//i++;
		}
		//std::cout << UNDERLINE "HERE 6: " << cpt << RESET << std::endl;
		//cpt++;
	}
	for(size_t i = 0; i < servers.size(); i++)
	{
		const std::vector<int> fds = servers[i].getSocketFds();
		for (size_t j = 0; j < fds.size(); j++)
			close(fds[j]);
	}
	for(size_t i = 0; i < client_fds_vec.size(); i++)
		close(client_fds_vec[i]);
	return 0;
}

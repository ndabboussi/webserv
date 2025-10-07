#include "Server.hpp"
#include <signal.h>

int	main(int ac, char **av)
{
	signal(SIGPIPE, SIG_IGN);
	std::string	configFile = "./conf/nanana.conf";
	if (ac > 2)
		return (std::cout << "Error: too many arguments." << std::endl, 1);
	else if (ac == 2)
		configFile = av[1];
	std::vector<Server>	servers;
	try
	{
		parsing(servers, configFile);
	}
	catch(const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
//	printServers(servers);
	launchServer(servers);
	return 0;
}

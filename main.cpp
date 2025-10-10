#include "Server.hpp"

volatile bool serverRunning = true;// Global flag that controls lauchServer() main loop

void handleSignal(int signum)
{
	if (signum == SIGINT)
	{
		std::cout << "\n\033[1;31m[!] Caught SIGINT — shutting down cleanly...\033[0m\n";
		serverRunning = false;
	}
}

int	main(int ac, char **av)
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handleSignal);
	std::string	configFile = "./conf/nanana.conf";
	if (ac > 2)
		return (std::cout << "Error: too many arguments." << std::endl, 1);
	else if (ac == 2)
		configFile = av[1];
	std::vector<Server>	servers;
	try
	{
		parsing(servers, configFile);
		debugPrintConfig(servers);
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

#include "Server.hpp"

void	printLocation(const std::vector<Location> &locations)
{
	int i = 0;
	for (std::vector<Location>::const_iterator it2 = locations.begin(); it2 != locations.end(); it2++, i++)
	{
		std::cout << "\n------location no " << i << "------" << std::endl << std::endl;
		std::cout << "Location Path: " << it2->getPath() << std::endl;
		std::cout << "Location methods: " << (int)it2->getMethods() << std::endl;
		std::cout << "-----location data-----" << std::endl;
		const std::map<std::string, std::string> data2 = it2->getData();
		for (std::map<std::string, std::string>::const_iterator it3 = data2.begin(); it3 != data2.end(); it3++)
			std::cout << "location " << it3->first << ": " << it3->second << std::endl;
		std::cout << "-----END location data-----" << std::endl;
		std::cout << "-----location Locations-----" << std::endl;
		printLocation(it2->getLocations());
		std::cout << "-----End location Locations-----" << std::endl;
	}
}

void	printServers(const std::vector<Server> &servers)
{
	int i = 0;
	for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); it++, i++)
	{
		
		std::cout << "\n------Server no " << i << "------" << std::endl << std::endl;
		std::cout << "server name: " << it->getName() << std::endl;
		//std::cout << "server port: " << it->getPorts() << std::endl;
		std::cout << "server methods: " << (int)it->getMethods() << std::endl;
		std::cout << "-----server data-----" << std::endl;
		const std::map<std::string, std::string> data = it->getData();
		for (std::map<std::string, std::string>::const_iterator it2 = data.begin(); it2 != data.end(); it2++)
			std::cout << "server " << it2->first << ": " << it2->second << std::endl;
		std::cout << "-----server Locations-----" << std::endl;
		printLocation(it->getLocations());
		std::cout << "-----END server Locations-----" << std::endl;
	}
}


void	printTokens(const std::vector<std::string> &tokens)
{
	std::cout << "=== printTokens ===" << std::endl;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::cout << "[" << i << "]: " << tokens[i] << std::endl;
	}
	std::cout << "==== end print ====" << std::endl;
}
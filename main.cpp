#include "Location.hpp"

int	main(int ac, char **av)
{
	std::string	configFile = "conf/nanana.conf";
	if (ac > 2)
		return (std::cout << "Error: too many arguments." << std::endl, 1);
	else
		configFile = av[1];
	try
	{
		//call parsing function
	}
	catch(const std::exception &e)
	{
		std::cerr << "Erreur: " << e.what() << '\n';
		return 1;
	}
	return 0;
}

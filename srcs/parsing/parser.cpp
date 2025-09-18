# include "Server.hpp"


static void	parsingLocation(Location &location, std::vector<std::string> file, std::vector<std::string>::iterator &it)
{
	for (; it != file.end() && *it != "}" ; it++)
	{

	}
}

static void	parsingServer(Server &server, std::vector<std::string> file, std::vector<std::string>::iterator &it)
{
	for (; it != file.end() && *it != "}" ; it++)
	{
		if (*it == "listen")
		{
			it++;
			if (isdigit((*it)[0]))
				server;
			else
				throw std::exception(); //missing port in field listen
			while (it != file.end() && *it != ";")
				it++;
		}
		if (*it == "server_name")
		{
			it++;
			
			while (it != file.end() && *it != ";")
				it++;
		}
	}
}

void	parsing(std::vector<Server> &servers, std::string configFile)
{
	// 1. file opening, if unable, throw and fuck off
	// 2. function : getline while oef, when keyword server or location is found, open dedicated function
	// 3. in parsingLocation and parsingServer 
	// 4. if eof and not in inital function, it means missing bracket = EXCEPTION
}

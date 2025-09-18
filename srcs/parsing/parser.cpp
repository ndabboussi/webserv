# include "Server.hpp"

static void	parsingLocation(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	for (; it != end && *it != "}" ; it++)
	{

	}
}

template<typename ServLoc>
void mapElement(ServLoc servLoc, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	std::string key, value;
	servLoc.addData();
}


static void	parsingServer(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	for (; it != end && *it != "}" ; it++)
	{
		if (*it == "location" && it + 1 != end && *(it + 1) == "{")
		{
			Location newLoc;
			it++;
			parsingLocation(newLoc, it, end);
			server.addLocations(newLoc);
		}
		if (*it == "listen" && it + 1 != end)
		{
			it++;
			if (isdigit((*it)[0]))
			{
				long nb = std::stol(*it);
				if (nb > 2147483648)
					throw std::exception(); //too large number in field listen
				//server.setPort(nb);
			}
			else if ((*it)[0] == ';')
				throw std::exception(); //missing port in field listen
			else
				throw std::exception(); //unrecognise char in field listen
			if (it + 1 != end && *(it + 1) == ";") //check if next str is a ;
				it++;
			if ((*it)[it->size() - 1] != ';')
				throw std::exception(); //missing ; or too much informations in instruction
		}
		else if (*it == "server_name" && it + 1 != end)
		{
			it++;
			if (*it != ";")
				;//server.setName(((*it)[it->size() - 1] == ';') ? it->substr(0, (*it)[it->size() - 2]): *it);
			else
				throw std::exception(); //missing server Name in field server_name
			if (it + 1 != end && *(it + 1) == ";")//check if next str is a ;
				it++;
			if ((*it)[it->size() - 1] != ';')
				throw std::exception(); //missing ; or too much informations in instruction
		}
		else if (*it != ";" && it + 1 != end)
			mapElement(server, end, it);
	}
}

void	parsing(std::vector<Server> &servers, std::string configFile)
{
	// 1. file opening, if unable, throw and fuck off
	// 2. function : getline while oef, when keyword server or location is found, open dedicated function
	// 3. in parsingLocation and parsingServer 
	// 4. if eof and not in inital function, it means missing bracket = EXCEPTION
}

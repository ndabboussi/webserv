# include "Server.hpp"

static void	parsingLocation(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	// for (; it != end && *it != "}" ; it++)
	// {

	// }
	(void)location;
	(void)end;
	(void)it;
}

// template<typename ServLoc>
// void mapElement(ServLoc servLoc, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
// {
// 	std::string key, value;
// 	servLoc.addData();
// }


static void	parsingServer(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	// for (; it != end && *it != "}" ; it++)
	// {
	// 	if (*it == "location" && it + 1 != end && *(it + 1) == "{")
	// 	{
	// 		Location newLoc;
	// 		it++;
	// 		parsingLocation(newLoc, it, end);
	// 		server.addLocations(newLoc);
	// 	}
	// 	if (*it == "listen" && it + 1 != end)
	// 	{
	// 		it++;
	// 		if (isdigit((*it)[0]))
	// 		{
	// 			long nb = std::stol(*it);
	// 			if (nb > 2147483648)
	// 				throw std::exception(); //too large number in field listen
	// 			//server.setPort(nb);
	// 		}
	// 		else if ((*it)[0] == ';')
	// 			throw std::exception(); //missing port in field listen
	// 		else
	// 			throw std::exception(); //unrecognise char in field listen
	// 		if (it + 1 != end && *(it + 1) == ";") //check if next str is a ;
	// 			it++;
	// 		if ((*it)[it->size() - 1] != ';')
	// 			throw std::exception(); //missing ; or too much informations in instruction
	// 	}
	// 	else if (*it == "server_name" && it + 1 != end)
	// 	{
	// 		it++;
	// 		if (*it != ";")
	// 			;//server.setName(((*it)[it->size() - 1] == ';') ? it->substr(0, (*it)[it->size() - 2]): *it);
	// 		else
	// 			throw std::exception(); //missing server Name in field server_name
	// 		if (it + 1 != end && *(it + 1) == ";")//check if next str is a ;
	// 			it++;
	// 		if ((*it)[it->size() - 1] != ';')
	// 			throw std::exception(); //missing ; or too much informations in instruction
	// 	}
	// 	else if (*it != ";" && it + 1 != end)
	// 		mapElement(server, end, it);
	// }
	(void)server;
	(void)it;
	(void)end;
}


static void tokenizeLine(const std::string &line, std::vector<std::string> &tokens)
{
	std::istringstream iss(line);
	std::string token;

	while (iss >> token)
		tokens.push_back(token);
}

static std::vector<std::string> tokenizeConfig(const std::string &configFile)
{
	std::ifstream file(configFile.c_str());
	if (!file.is_open())
		throw std::runtime_error("Error: cannot open config file: " + configFile);

	std::vector<std::string> tokens;
	std::string line;

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;
		tokenizeLine(line, tokens);
	}

	return tokens;
}

void parsing(std::vector<Server> &servers, std::string configFile)
{
	std::vector<std::string> tokens = tokenizeConfig(configFile);
	printTokens(tokens); //debug

	int flagBrackets = 0;
	std::vector<std::string>::iterator end = tokens.end();
	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)
	{
		if (*it == "{")
			flagBrackets++;
		else if (*it == "}")
			flagBrackets--;
		if (flagBrackets < 0)
			throw std::runtime_error("Error: unmatched closing bracket in config file");

		if (*it == "server")
		{
			Server server;
			
			parsingServer(server, it, end);
			servers.push_back(server);
		}
		else if (*it == "location")
		{
			if (servers.empty())
				throw std::runtime_error("Error: 'location' outside of a 'server' block");

			Location loc;
			parsingLocation(loc, it, end);
			servers.back().addLocation(loc);
		}
	}

	if (flagBrackets != 0)
		throw std::runtime_error("Error: unmatched brackets in config file");
}
# include "Server.hpp"
# include "parser.tpp"

static void fillLocation(Location &dest, Location src)
{
	dest.cpyData(src.getData());
	dest.setMethods(src.getMethods());
}

static void	parsingLocation(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	for (; it != end && *it != "}" ; it++)
	{
		if (*it == "location" && it + 1 != end && it + 2 != end && *(it + 2) == "{")
		{
			Location newLoc;
			fillLocation(newLoc, location);
			newLoc.setPath(*(it + 1));
			it += 3;
			parsingLocation(newLoc, it, end);
			location.addLocations(newLoc);
		}
		else if ((*it == "autoindex" || *it == "autoindex;") && it + 1 != end)
		{
			location.setAutoIndex(true);
			if ((*it)[it->size() - 1] != ';' && *(++it) != ";")
		 		throw std::runtime_error("Error: Missing ; in location scope"); // missing ;
		}
		else if (*it == "allow_methods" && it + 1 != end)
			setMethods(location, ++it, end);
		else if (*it != ";" && it + 1 != end)
			mapElement(location, it, end);
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in location scope"); //Unexpexted EOF
}

static void	parsingServer(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	for (; it != end && *it != "}" ; it++)
	{
		if (*it == "location" && it + 1 != end && it + 2 != end && *(it + 2) == "{")
		{
			Location newLoc;
			fillLocation(newLoc, server);
			newLoc.setPath(*(it + 1));
			it += 3;
			parsingLocation(newLoc, it, end);
			server.addLocations(newLoc);
		}
		else if (*it == "allow_methods" && it + 1 != end)
			setMethods(server, ++it, end);
		else if (*it == "listen" && it + 1 != end)
		{
			it++;
			if (isdigit((*it)[0]))
			{
				long nb = std::atol(it->c_str());
				if (nb > 2147483648)
					throw std::runtime_error("Error: Too large number in field listen"); //too large number in field listen
				server.setPort(nb);
			}
			else if ((*it)[0] == ';')
				throw std::runtime_error("Error: Missing port in field listen"); //missing port in field listen
			else
				throw std::runtime_error("Error: Unrecognised char in field listen"); //unrecognise char in field listen
			if (it + 1 != end && *(it + 1) == ";") //check if next str is a ;
				it++;
			if ((*it)[it->size() - 1] != ';')
				throw std::runtime_error("Error: Error: Missing ; or too much informations in instruction in listen field in server scope"); //missing ; or too much informations in instruction
		}
		else if (*it == "server_name" && it + 1 != end)
		{
			it++;
			if (*it != ";")
				server.setName(((*it)[it->size() - 1] == ';') ? it->substr(0, it->size() - 2): *it);
			else
				throw std::runtime_error("Error: Missing server Name in field server_name in server scope"); //missing server Name in field server_name
			if (it + 1 != end && *(it + 1) == ";")//check if next str is a ;
				it++;
			if ((*it)[it->size() - 1] != ';')
				throw std::runtime_error("Error: Missing ; or too much informations in instruction in server_name field in server scope");
		}
		else if (*it != ";" && it + 1 != end)
			mapElement(server, it, end);
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in server scope");
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
	//printTokens(tokens); //debug

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

		if (*it == "server" && it + 1 != end && *(it + 1) == "{")
		{
			Server server;
			
			it += 2;
			parsingServer(server, it, end);
			servers.push_back(server);
		}
		// else if (*it == "location" && it + 1 != end && *(it + 1) == "{")
		// {
		// 	if (servers.empty())
		// 		throw std::runtime_error("Error: 'location' outside of a 'server' block");

		// 	it += 2;
		// 	Location loc;
		// 	parsingLocation(loc, it, end);
		// 	servers.back().addLocations(loc);
		// }
	}

	if (flagBrackets != 0)
		throw std::runtime_error("Error: unmatched brackets in config file");
}
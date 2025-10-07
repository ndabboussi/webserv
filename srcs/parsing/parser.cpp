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
		else if ((*it == "auto_index" || *it == "auto_index;") && it + 1 != end)
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

static int isStrDigit(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return 0;
	}
	return 1;
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
			int nb = 0;
			if (isStrDigit(*it) || (isStrDigit(it->substr(0, it->size() - 1)) && (*it)[it->size() - 1] == ';'))
			{
				while (it != end && ( isStrDigit(*it) || (isStrDigit(it->substr(0, it->size() - 1)) && (*it)[it->size() - 1] == ';')))
				{
					nb++;
					long nb = std::atol(it->c_str());
					if (nb > 2147483648)
						throw std::runtime_error("Error: Too large number in field listen"); //too large number in field listen
					server.addPort(nb);
					it++;
				}
				if (it == end)
					throw std::runtime_error("Error: Unexpected EOF");
			}
			else if ((*it)[0] == ';' && !nb)
				throw std::runtime_error("Error: Missing port in field listen"); //missing port in field listen
			else
				throw std::runtime_error("Error: Unrecognised char in field listen"); //unrecognise char in field listen
			if ((*(it - 1))[(it - 1)->size() - 1] == ';') //check if next str is a ;
				it--;
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
		else if (*it == "max_client_body_size" && it + 1 != end)
		{
			it++;
			if (isdigit((*it)[0]))
			{
				double nb = std::atof(it->c_str());
				if (nb > static_cast<double>(9223372036854775807))
					throw std::runtime_error("Error: Too large number in field max_body_client_size");
				server.setMaxBodyClientSize(static_cast<long long>(nb));
			}
			else if ((*it)[0] == ';')
				throw std::runtime_error("Error: Missing port in field max_body_client_size");
			else
				throw std::runtime_error("Error: Unrecognised char in field max_body_client_size");
			if (it + 1 != end && *(it + 1) == ";") //check if next str is a ;
				it++;
			if ((*it)[it->size() - 1] != ';')
				throw std::runtime_error("Error: Error: Missing ; or too much informations in instruction in maxClientBodySize field in server scope");
		}
		else if (*it == "error_page" && it + 1 != end)
		{
			it++;
			int key;
			std::string value;
			if (isStrDigit(*it) && it + 1 != end)
			{
				key = std::atoi(it->c_str());
				it++;
				if ((*it)[it->size() - 1] == ';' && it->size() > 1)
					server.addErrorPage(key, it->substr(0, it->size() - 1));
				else
				{
					server.addErrorPage(key, *it);
					it++;
				}
				if ((*it)[it->size() - 1] != ';')
					throw std::runtime_error("Error: Error: Missing ; or too much informations in instruction in error_page field in server scope");
			}
			else if ((*it)[0] == ';')
				throw std::runtime_error("Error: Missing error number in field error_page");
			else
				throw std::runtime_error("Error: Unrecognised char in field error_page");
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
	}
	if (flagBrackets != 0)
		throw std::runtime_error("Error: unmatched brackets in config file");
}
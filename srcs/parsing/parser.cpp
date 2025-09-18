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
	for (std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); it++)
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
			parsingServer(server, tokens, it);
			servers.push_back(server);
		}
		else if (*it == "location")
		{
			if (servers.empty())
				throw std::runtime_error("Error: 'location' outside of a 'server' block");

			Location loc;
			parsingLocation(loc, tokens, it);
			servers.back().addLocation(loc);
		}
	}

	if (flagBrackets != 0)
		throw std::runtime_error("Error: unmatched brackets in config file");
}
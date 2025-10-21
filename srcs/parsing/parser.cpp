# include "parserConfig.hpp"

void fillLocation(Location &dest, Location src)
{
	dest.cpyData(src.getData());
	dest.setMethods(src.getMethods());
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
			std::map<std::string, std::string> map = servers[servers.size() - 1].getData();
			if (map.count("root") == 0)
				servers[servers.size() - 1].addData("root", "/conf/data/test");
		}
	}
	if (flagBrackets != 0)
		throw std::runtime_error("Error: unmatched brackets in config file");
}
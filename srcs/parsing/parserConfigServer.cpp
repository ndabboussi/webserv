# include "parserConfig.hpp"

static int isStrDigit(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]))
			return 0;
	}
	return 1;
}

static void fillPort(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
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

static void fillServerName(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	if (*it != ";")
		server.setName(((*it)[it->size() - 1] == ';') ? it->substr(0, it->size() - 2): *it);
	else
		throw std::runtime_error("Error: Missing server Name in field server_name in server scope"); //missing server Name in field server_name
	if (it + 1 != end && *(it + 1) == ";")//check if next str is a ;
		it++;
	if ((*it)[it->size() - 1] != ';')
		throw std::runtime_error("Error: Missing ; or too much informations in instruction in server_name field in server scope");
}

static void fillBodyClientSize(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
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

static void fillErrorPage(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
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

static void addNewLocation(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	Location newLoc;

	fillLocation(newLoc, server);
	newLoc.setPath(*(it + 1));
	it += 3;
	parsingLocation(newLoc, it, end);
	server.addLocations(newLoc);
}

void	parsingServer(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	for (; it != end && *it != "}" ; it++)
	{
		if (*it == "location" && it + 1 != end && it + 2 != end && *(it + 2) == "{")
			addNewLocation(server, it, end);
		else if (*it == "allow_methods" && it + 1 != end)
			setMethods(server, ++it, end);
		else if (*it == "listen" && it + 1 != end)
			fillPort(server, ++it, end);
		else if (*it == "server_name" && it + 1 != end)
			fillServerName(server, ++it, end);
		else if (*it == "max_client_body_size" && it + 1 != end)
			fillBodyClientSize(server, ++it, end);
		else if (*it == "error_page" && it + 1 != end)
			fillErrorPage(server, ++it, end);
		else if (*it != ";" && it + 1 != end)
			mapElement(server, it, end);
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in server scope");
}

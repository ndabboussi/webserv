#include "parserConfig.hpp"

static void	addNewLocation(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	Location newLoc;

	fillLocation(newLoc, location);
	newLoc.setPath(*(it + 1));
	it += 3;
	parsingLocation(newLoc, it, end);
	location.addLocations(newLoc);
}

static void fillCgiPath(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	while (it != end)
	{
		std::string	token = *it;
		bool endOfLine = (token[token.size() - 1] == ';');

		if (endOfLine)
			token = token.substr(0, token.size() - 1);
		if (!token.empty())
			location.addCgiPath(token);
		if (endOfLine)
			break;
		it++;
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in cgi_path field");
}

static void fillCgiExt(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	while (it != end)
	{
		std::string token = *it;
		bool endOfLine = (token[token.size() - 1] == ';');

		if (endOfLine)
			token = token.substr(0, token.size() - 1);

		if (!token.empty())
			location.addCgiExt(token); // <-- assumes addCgiExt() stores multiple extensions
		if (endOfLine)
			break;
		it++;
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in cgi_ext field");
}

static void fillAutoIndex(Location &location, std::vector<std::string>::iterator &it)
{
	location.setAutoIndex(true);
	if ((*it)[it->size() - 1] != ';' && *(++it) != ";")
		throw std::runtime_error("Error: Missing ; in location scope"); // missing ;
}

void	parsingLocation(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	for (; it != end && *it != "}" ; it++)
	{
		if (*it == "location" && it + 1 != end && it + 2 != end && *(it + 2) == "{")
			addNewLocation(location, it, end);
		else if (*it == "cgi_path" && it + 1 != end)
			fillCgiPath(location, ++it, end);
		else if (*it == "cgi_ext" && it + 1 != end)
			fillCgiExt(location, ++it, end);
		else if ((*it == "auto_index" || *it == "auto_index;") && it + 1 != end)
			fillAutoIndex(location, it);
		else if (*it == "allow_methods" && it + 1 != end)
			setMethods(location, ++it, end);
		else if (*it != ";" && it + 1 != end)
			mapElement(location, it, end);
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in location scope"); //Unexpexted EOF
}

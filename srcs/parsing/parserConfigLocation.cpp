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

static void setRedirect(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end, int &count)
{
	int key;
	if (count > 1)
		throw std::runtime_error("Error: There must be only return in location block if return is present in location bloc");
	if (isStrDigit(*it) && it + 1 != end)
	{
		key = std::atoi(it->c_str());
		if (key != 301 && key != 302)
			throw std::runtime_error("Error: redirection code in return field neither 301 or 302");
		it++;
		if ((*it)[it->size() - 1] == ';' && it->size() > 1)
			location.setRedirect(key, it->substr(0, it->size() - 1));
		else
		{
			location.setRedirect(key, *it);
			it++;
		}
		if ((*it)[it->size() - 1] != ';')
			throw std::runtime_error("Error: Missing ; or too much informations in instruction in return field in location scope");
	}
	else if ((*it)[0] == ';')
		throw std::runtime_error("Error: Missing Redirection number in field return");
	else
		throw std::runtime_error("Error: Unrecognised char in field return");
}

void	parsingLocation(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	int count = 0;

	for (; it != end && *it != "}" ; it++)
	{
		if (*it == "location" && it + 1 != end && it + 2 != end && *(it + 2) == "{" && ++count)
			addNewLocation(location, it, end);
		else if (*it == "return" && it + 1 != end && ++count)
			setRedirect(location, ++it, end, count);
		else if (*it == "cgi_path" && it + 1 != end && ++count)
			fillCgiPath(location, ++it, end);
		else if (*it == "cgi_ext" && it + 1 != end && ++count)
			fillCgiExt(location, ++it, end);
		else if ((*it == "auto_index" || *it == "auto_index;") && it + 1 != end && ++count)
			fillAutoIndex(location, it);
		else if (*it == "allow_methods" && it + 1 != end && ++count)
			setMethods(location, ++it, end);
		else if (*it != ";" && it + 1 != end && ++count)
			mapElement(location, it, end);
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in location scope"); //Unexpexted EOF
}

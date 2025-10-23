#ifndef PARSERCONFIG_HPP
# define PARSERCONFIG_HPP

# include "Server.hpp"
# include "parser.tpp"

void	parsingServer(Server &server, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
void	parsingLocation(Location &location, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end);
void	fillLocation(Location &dest, Location src);
int		isStrDigit(std::string str);

#endif
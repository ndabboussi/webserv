# include "Server.hpp"

template<typename ServLoc>
void mapElement(ServLoc &servLoc, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	std::string key, value;
	key = *it;
	it++;
	if (*it != ";")
		value = ((*it)[it->size() - 1] == ';') ? it->substr(0, it->size() - 1): *it;
	else
		throw std::runtime_error("Error: Missing information"); //missing server Name in field server_name
	if (it + 1 != end && *(it + 1) == ";")//check if next str is a ;
		it++;
	if ((*it)[it->size() - 1] != ';')
		throw std::runtime_error("Error: Missing ; or too much informations in instruction"); //missing ; or too much informations in instruction
	servLoc.addData(key, value);
}

template<typename ServLoc>
void setMethods(ServLoc &servLoc, std::vector<std::string>::iterator &it, std::vector<std::string>::iterator end)
{
	uint8_t nb = 0;
	for (; it != end && *it != ";"; it++)
	{
		std::string str = ((*it)[it->size() - 1] == ';') ? it->substr(0, it->size() - 1): *it;
		if (str == "GET")
			nb |= GET;
		else if (str == "POST")
			nb |= POST;
		else if (str == "DELETE")
			nb |= DELETE;
		else if (str != "GET" && str != "POST" && str != "DELETE")
			throw std::runtime_error("Error: Unrecognised method: " + str); //Unrecognise method
		else
			throw std::runtime_error("Error: Method is used twice: " + str); //method is used twice
		if ((*it)[it->size() - 1] == ';')
			break ;
	}
	if (it == end)
		throw std::runtime_error("Error: Unexpected EOF in set methods scope"); //Unexpexted EOF
	servLoc.setMethods(nb);
}
#include "Server.hpp"


void	printTokens(const std::vector<std::string> &tokens)
{
	std::cout << "=== printTokens ===" << std::endl;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::cout << "[" << i << "]: " << tokens[i] << std::endl;
	}
	std::cout << "==== end print ====" << std::endl;
}
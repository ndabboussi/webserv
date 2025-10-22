#include "Server.hpp"
#include "Client.hpp"

#include <iostream>
#include <iomanip>

#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define BLUE   "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN   "\033[36m"
#define RESET  "\033[0m"
#define BOLD "\033[1m"

// =============================================================
// CONFIG PARSING DEBUG
// =============================================================
void debugPrintConfig(const std::vector<Server>& servers)
{
	std::cout << "\n" << MAGENTA << "========== CONFIG PARSING RESULT ==========" << RESET << "\n";

	for (size_t i = 0; i < servers.size(); ++i)
	{
		const Server& srv = servers[i];
		std::cout << BOLD << BLUE << "\nServer [" << i << "]" << RESET << "\n";
		std::cout << "  Name: " << srv.getName() << "\n";

		std::cout << "  Ports: ";
		for (size_t p = 0; p < srv.getPorts().size(); ++p)
			std::cout << srv.getPorts()[p] << " ";
		std::cout << "\n";

		std::cout << "  MaxBodySize: " << srv.getMaxBodyClientSize() << "\n";

		const std::map<std::string, std::string> data = srv.getData();
		std::cout << "  Data:\n";
		for (std::map<std::string, std::string>::const_iterator it = data.begin(); it != data.end(); ++it)
			std::cout << "    " << it->first << " : " << it->second << "\n";

		const std::vector<Location>& locs = srv.getLocations();
		std::cout << "  Locations (" << locs.size() << "):\n";

		for (size_t j = 0; j < locs.size(); ++j)
		{
			const Location& loc = locs[j];
			std::cout << "    ├── Path: " << GREEN << loc.getPath() << RESET << "\n";
			std::cout << "    │   Methods: " << (int)loc.getMethods() << "\n";
			std::cout << "    │   AutoIndex: " << (loc.getAutoIndex() ? "on" : "off") << "\n";

			std::cout << "    │   Data:\n";
			const std::map<std::string, std::string>& d = loc.getData();
			for (std::map<std::string, std::string>::const_iterator it = d.begin(); it != d.end(); ++it)
				std::cout << "    │     " << it->first << " : " << it->second << "\n";

			std::cout << "    │   CGI Paths (" << loc.getCgiPath().size() << "): ";
			for (size_t k = 0; k < loc.getCgiPath().size(); ++k)
				std::cout << loc.getCgiPath()[k] << " ";
			std::cout << "\n";

			std::cout << "    │   CGI Exts (" << loc.getCgiExt().size() << "): ";
			for (size_t k = 0; k < loc.getCgiExt().size(); ++k)
				std::cout << loc.getCgiExt()[k] << " ";
			std::cout << "\n";
		}
	}

	std::cout << MAGENTA << "\n===========================================\n" << RESET;
}


// =============================================================
// HTTP REQUEST DEBUG
// =============================================================
void debugPrintRequest(const HttpRequest& req)
{
	std::cout << "\n" << CYAN << "========== HTTP REQUEST PARSING RESULT ==========" << RESET << "\n";
	std::cout << "StatusCode: " << req.statusCode << "\n";
	std::cout << "Method:     " << req.method << "\n";
	std::cout << "Path:       " << req.path << "\n";
	std::cout << "Version:    " << req.version << "\n";
	std::cout << "URL:        " << req.url << "\n";
	std::cout << "isCgi:      " << (req.isCgi ? "true" : "false") << "\n";
	std::cout << "methodPath: " << (int)req.methodPath << "\n";
	std::cout << "autoIndexFile size: " << req.autoIndexFile.size() << "\n";

	std::cout << "\nHeaders:\n";
	for (std::map<std::string, std::string>::const_iterator it = req.header.begin(); it != req.header.end(); ++it)
		std::cout << "  " << it->first << ": " << it->second << "\n";

	std::cout << "\nBody:\n";
	for (std::map<std::string, std::string>::const_iterator it = req.body.begin(); it != req.body.end(); ++it)
		std::cout << "  " << it->first << " = " << it->second << "\n";

	std::cout << CYAN << "==============================================\n" << RESET;
}

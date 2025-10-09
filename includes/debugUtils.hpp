#ifndef DEBUGUTILS_HPP
# define DEBUGUTILS_HPP

# include "Server.hpp"
# include "parsingRequest.hpp"

struct HttpRequest;

void debugPrintConfig(const std::vector<Server>& servers);
void debugPrintRequest(const HttpRequest& req);

#endif

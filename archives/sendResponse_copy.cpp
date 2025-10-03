// void	sendResponse(int client_fd, const HttpRequest &request)
// {
// 	HttpResponse	resp;
// 	setStatusCode(request, resp);
// 	setStatusLine(resp);

// 	// Step 1: Check if request already has an error
// 	if (request.statusCode)
// 	{
// 		buildErrorBody(client_fd, request.statusCode);
// 		return;
// 	}
// 	//// Step 2: CGI (skip building full response here)
// 	// if (_cgi)
// 	// 	return;

// 	// Step 3: Autoindex case
// 	else if (!request.autoIndexFile.empty())
// 	{
// 		std::string autoIndexBody= request.autoIndexFile;

// 		std::ostringstream response;
// 		response << resp.statusLine;
// 		response << "Date: " << setDate();
// 		response << "Server: MyWebServ/1.0\r\n";
// 		response << "Connection: " << setConnection(request);
// 		response << "Content-Type: text/html\r\n";
// 		response << "Content-Length: " << autoIndexBody.size() << "\r\n";
// 		response << "\r\n"; // end of headers
// 		response << autoIndexBody;

// 		std::string respStr = response.str();
// 		send(client_fd, respStr.c_str(), respStr.size(), 0);

// 		std::cout << GREEN "[<] Sent Autoindex Response for: " 
// 					<< request.path << RESET << std::endl;
// 		return;
// //403 Forbidden : if autoindex is deativated in config but is asked by client and no index.html found
// //404 Not Found : if the directory doesnt exist at all
// 	}

// 	// // Step 4: Empty file → 204 No Content
// 	// else if (response.code == 200 && _response_body.empty() && fileExists(_target_file))
// 	// {
// 	// 	struct stat st;
// 	// 	if (stat(_target_file.c_str(), &st) == 0 && st.st_size == 0)
// 	// 	{
// 	// 		response.code = 204;
// 	// 		_response_body.clear();
// 	// 	}
// 	// }

// 	// // Step 5: POST created new file → 201 Created
// 	// if (request.getMethod() == POST && response.code == 200 && !fileExists(_target_file))
// 	// {
// 	// 	response.code = 201;
// 	// }

// 	std::ifstream file(request.path.c_str(), std::ios::binary);
// 	std::vector<char> fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
// 	std::ostringstream response;
// 	response << resp.statusLine;
// 	response << "Date: " << setDate();
// 	response << "Server: " << "MyWebServ/1.0\r\n";
// 	response << "Connection: " << setConnection(request);
// 	// if (_code != 204) // No Content → no Content-Type/Length
// 	// {
// 	// 	contentType();
// 	// 	contentLength();
// 	// }
// 	response << "Content-Type: " << getContentType(request.path) << "\r\n";
// 	response << "Content-Length: " << fileContent.size() << "\r\n";
// 	response << "\r\n";

// 	std::string headers = response.str();
// 	send(client_fd, headers.c_str(), headers.size(), 0);
// 	send(client_fd, fileContent.data(), fileContent.size(), 0);

// 	std::cout << GREEN "[<] Sent Response:\n" << headers.c_str() << RESET;
// 	std::cout << GREEN "[<] Sent file: " << request.path
// 				<< " (" << fileContent.size() << " bytes)" << RESET << std::endl;
// }



// void	buildErrorBody(int client_fd, int code)
// {
// 	std::string	statusMessage = statusCodeResponse(code);
// 	std::string body = generateDefaultErrorPage(code);

// 	std::ostringstream response;
// 	response << "HTTP/1.1 " << code << " " << statusMessage << "\r\n";
// 	response << "Date: " << setDate();
// 	response << "Server: " << "MyWebServ/1.0\r\n";
// 	response << "Content-Type: text/html\r\n";
// 	response << "Content-Length: " << body.size() << "\r\n";
// 	//response << "Connection: " << setConnection(request);
// 	response << "Connection: close\r\n\r\n";
// 	response << body;
// 	std::string resp = response.str();
// 	send(client_fd, resp.c_str(), resp.size(), 0);
// 	std::cout << GREEN "[<] Sent Response:\n" << resp.c_str() << RESET << std::endl;
// 	return;
// }

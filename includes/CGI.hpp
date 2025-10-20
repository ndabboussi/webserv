#ifndef CGI_HPP
# define CGI_HPP

# include "Server.hpp"
# include <sys/wait.h>
// #include <fcntl.h>
// #include <sstream>
// #include <cstdlib>
// #include <cstring>
// #include <cerrno>
// #include <iostream>

# include <stdexcept>

struct HttpRequest;
struct Context;

enum	CgiType
{
	BINARY,
	PYTHON,
	PERL,
	PHP,
	SHELL,
	JS,
	HTML,
	CSS,
	UNKNOWN
};

class	CGI
{
	private:
		HttpRequest							&_request;
		std::string							_extension;
		std::string							_path;
		std::map<std::string, std::string> 	_defaults;;
		std::string							_interpreter;
		int									_type;
		std::vector<std::string>			_env;
		std::vector<char*>					_envp;

	private:
		std::vector<std::string>	_buildCgiEnv(const HttpRequest &req, const Server &server, const std::string &scriptPath) const;
		std::vector<char*>			_envVecToCharPtr(const std::vector<std::string> &env) const;

		std::string					_readFromFd(int fd) const;
		std::string					_parseCgiOutput(const std::string &raw, int &outStatusCode, std::map<std::string,std::string> &outHeaders) const;
		std::vector<std::string>	_split(const std::string &s) const;
		bool						_postSupported() const;

	public:
		CGI();
		CGI(HttpRequest &request);
		CGI &operator=(CGI const &src);
		CGI(CGI const &src);
		~CGI();

	public:
		std::string					executeCgi(const HttpRequest &request, Server &server, int clientFd, Context &context);

		void						setCgiInfos(const HttpRequest &request, const Server &server);
		int 						getCgiType() const;
		std::string  				setExtension() const;
		std::string  				getExtension() const;
		std::string  				getPath() const;
		int							checkAccess() const;

	// class CGIException : public std::exception
	// {
	// 	private:
	// 		std::string	_message;
	// 		int			_status;

	// 	public:
	// 		void CGIexception(std::string error)
	// 		{
	// 			this->_message = "[CGI ERROR] " + error; 
	// 		}
	// 		virtual const char *what() const throw()
	// 		{
	// 			return (_message.c_str());
	// 		}
	// };
};

#endif

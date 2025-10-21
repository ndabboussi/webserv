#ifndef CGI_HPP
# define CGI_HPP

# include "Server.hpp"
# include <sys/wait.h>

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
		Client								&_client;
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
		CGI(HttpRequest &request, Client &client);
		CGI &operator=(CGI const &src);
		CGI(CGI const &src);
		~CGI();

	public:
		std::string					executeCgi(HttpRequest &request, Server &server, int clientFd, Context &context);

		void						setCgiInfos(const HttpRequest &request, const Server &server);
		int 						getCgiType() const;
		std::string  				setExtension() const;
		std::string  				getExtension() const;
		std::string  				getPath() const;
		int							checkAccess() const;
};

#endif

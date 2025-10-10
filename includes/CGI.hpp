#ifndef CGI_HPP
# define CGI_HPP

# include "Server.hpp"

struct HttpRequest;

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
		std::string							_extension;
		std::string							_path;
		std::map<std::string, std::string> 	_defaults;;
		std::string							_interpreter;
		int									_type;
		std::vector<std::string>			_env;
		std::vector<char*>					_envp;

	private:
		void						_setCgiInfos(const HttpRequest &request, const Server &server);
		int 						_getCgiType() const;
		std::string					_getExtension() const;
		int							_checkAccess() const;

		std::vector<std::string>	_buildCgiEnv(const HttpRequest &req, const Server &server, const std::string &scriptPath) const;
		std::vector<char*>			_envVecToCharPtr(const std::vector<std::string> &env) const;

		std::string					_readFromFd(int fd) const;
		std::string					_parseCgiOutput(const std::string &raw, int &outStatusCode, std::map<std::string,std::string> &outHeaders) const;
		std::vector<std::string>	_split(const std::string &s) const;

	public:
		CGI();
		~CGI();

	public:
		std::string	executeCgi(const HttpRequest &request, const Server &server);
};

#endif

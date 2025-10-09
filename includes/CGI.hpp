#ifndef CGI_HPP
# define CGI_HPP

# include "Server.hpp"

struct HttpRequest;

class	CGI
{
	private:
		std::vector<std::string>	_buildCgiEnv(const HttpRequest &req, const Server &server, const std::string &scriptPath) const;
		std::vector<char*>			_envVecToCharPtr(const std::vector<std::string> &env) const;
		void freeEnvCharVec(std::vector<char*> &vec) const;

		std::string					_readFromFd(int fd) const;
		std::string					_parseCgiOutput(const std::string &raw, int &outStatusCode, std::map<std::string,std::string> &outHeaders) const;
		std::vector<std::string>	_split(const std::string &s) const;
		int							_checkAccess(const std::string &path, int type);

		int 						_getCgiType(const std::string &ext) const;
		std::string					_getExtension(const std::string &path) const;

	public:
		CGI();
		~CGI();

	public:
		std::string	getCgiInterpreter(const std::string &ext, const Server &server) const;
		std::string	executeCgi(const HttpRequest &request, const Server &server);
		//int			executeCgi(const HttpRequest &request, const Server &server);
	
	// class CGIException : public std::exception
	// {
	// 	private:
	// 		std::string 	_message;
	// 		int				_exit;
	// 		unsigned int	_http_status;
	// 		std::string		_serverUid;
	// 	public:
	// 		CGIException() throw()
	// 		{
	// 		}
	// 		virtual const char* what() const throw()
	// 		{
	// 			return ();
	// 		}
	// };
};

#endif

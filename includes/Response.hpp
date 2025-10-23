#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <sys/stat.h>
# include "Server.hpp"

struct	Context;
class	Client;

class	Server;
struct	HttpRequest;

enum	MimeCategory
{
	APPLICATION,
	AUDIO,
	IMAGE,
	MULTIPART,
	TEXT,
	VIDEO,
	VND,
	UNKNOWN_MIME
};

class	Response
{
	private:
		Server								&_server;
		HttpRequest							&_request;
		int									_clientFd;

		int									_code;
		std::string							_statusLine;

		std::ostringstream					_headerStream;
		std::ostringstream					_headersFinal;
		std::string							_contentType;
		std::string							_contentLenght;
		std::string							_cookies;
		std::string							_connection;
		std::string							_body;
		std::string							_errorMsg;

	public:
		Response(int clientFd, HttpRequest &req, Server &server);
		Response(Response const &src);
		Response &operator=(Response const &src);
		~Response();

	public:
		void		setStatusCode();
		void		setStatusLine();

		void		setHeader(const std::string &key, const std::string &value);
		void		setBody(const std::string &body, const std::string &type);
		void		sendTo();
		void		appendCookies(std::ostringstream &res);
		std::string	build();

		bool		errorResponse();
		bool		autoIndexResponse();
		bool		redirectResponse();
		bool		cgiResponse(Client &client, Context &context);
		bool		postMethodResponse();
		std::string	buildPostConfirmation();
		bool		fileResponse();
};

std::string 	statusCodeResponse(int code);
std::string		getContentType(const std::string &path);
MimeCategory	getMimeCategory(const std::string &path);
std::string		setConnection(HttpRequest const &request);
std::string		setDate();

# endif

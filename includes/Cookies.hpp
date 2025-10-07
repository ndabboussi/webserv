#ifndef COOKIES_HPP

# define COOKIES_HPP

# include "Server.hpp"

struct HttpRequest;

class Cookies
{
	private:
		std::string					_id;
		std::string					_prevId;
		std::string					_authToken;
		int							_modified;
		std::vector<std::string>	_outputData;

	public:
		Cookies(void);
		Cookies(Cookies const &src);
		Cookies &operator=(Cookies const &src);
		~Cookies(void);

	public:
		std::string					getId(void) const;
		std::string					getPrevId(void) const;
		std::string					getAuth(void) const;
		int							getModified(void) const;
		std::vector<std::string>	getOutputData(void) const;

		void						setId(std::string value);
		void						setPrevId(std::string value);
		void						setAuth(std::string value);
		void						setModified(int index);
		void						addOutputData(std::string value);

		static std::string			genCookieId(std::vector<Cookies> const &cookies, int n);
};

void	manageCookies(Server &server, HttpRequest &request);

#endif
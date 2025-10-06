#ifndef COOKIES_HPP

# define COOKIES_HPP

# include "Server.hpp"

struct HttpRequest;


class Cookies
{
	private:
		std::string													_id;
		std::map<std::string, std::map<std::string, std::string> >	_data;
		int															_modified;
		std::map<std::string, std::string>							_outputData;

	public:
		Cookies(void);
		Cookies(Cookies const &src);
		Cookies &operator=(Cookies const &src);
		~Cookies(void);

	public:
		std::string													getId(void) const;
		std::map<std::string, std::map<std::string, std::string> >	getData(void) const;
		int															getModified(void) const;
		std::map<std::string, std::string>							getOutputData(void) const;

		void														addData(std::string path, std::string key, std::string value);
		void														setModified(int index);
		void														addOutputData(std::string key, std::string value);

		void														genCookieId(std::vector<Cookies> const &cookies);
};

void	manageCookies(Server &server, HttpRequest &request);

#endif
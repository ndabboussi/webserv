# include "Server.hpp"

//Operators----------------------------------------------------------------

Cookies &Cookies::operator=(Cookies const &src)
{
    if (this != &src)
    {
    	this->_id = src._id;
		this->_prevId = src._prevId;
		this->_authToken = src._authToken;
		this->_modified = src._modified;
		this->_outputData = src._outputData;
		this->_prevAuthToken = src._prevAuthToken;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Cookies::Cookies(void): _modified(-1){}

Cookies::Cookies(Cookies const &src): _id(src._id), _prevId(src._prevId), _prevAuthToken(src._prevAuthToken), _authToken(src._authToken), _modified(src._modified), _outputData(src._outputData)
{}

Cookies::~Cookies(void)
{}

//GETTERS------------------------------------------------------------------

std::string	Cookies::getId(void) const
{
	return this->_id;
}

std::string	Cookies::getPrevId(void) const
{
	return this->_prevId;
}

std::string Cookies::getAuth(void) const
{
	return this->_authToken;
}

std::string Cookies::getPrevAuthToken(void) const
{
	return this->_prevAuthToken;
}

int Cookies::getModified(void) const
{
	return this->_modified;
}

std::vector<std::string> Cookies::getOutputData(void) const
{
	return this->_outputData;
}

//SETTERS------------------------------------------------------------------

void Cookies::setModified(int index)
{
	this->_modified = index;
}

void Cookies::setId(std::string value)
{
	this->_id = value;
}

void Cookies::setPrevId(std::string value)
{
	this->_prevId = value;
}

void Cookies::setAuth(std::string value)
{
	this->_authToken = value;
}

void Cookies::setPrevAuthToken(std::string value)
{
	this->_prevAuthToken = value;
}

void Cookies::addOutputData(std::string value)
{
	this->_outputData.push_back(value);
}

void Cookies::delOutputData(int index)
{
	if (index < 0 || index > static_cast<int>(this->_outputData.size()))
		return;
	this->_outputData.erase(this->_outputData.begin() + index);
}

//Member function----------------------------------------------------------

std::string Cookies::genCookieId(std::vector<Cookies> const &cookies, int nb)
{
	int					file = open("/dev/urandom", O_RDONLY);
	std::vector<unsigned char>	rd(nb);
	std::string			dest;

	if (file < 0)
		throw std::runtime_error("Error: cannot open file");
	while (1)
	{
		int total = 0;
		while (total != nb)
		{
			int n = read(file, rd.data() + total, nb - total);
			if (n < 0)
			{
				close(file);
				throw std::runtime_error("Error: Read function failed");
			}
			total += n;
		}
		for (int i = 0; i < nb; i++)
		{
			if (rd[i] > 126)
				rd[i] %= 127;
			if (rd[i] < 33)
				rd[i] += 33;
			if (rd[i] == ',' || rd[i] == ';')
				rd[i] -= 1;
			dest += rd[i];
		}
		size_t i;
		for (i = 0; i < cookies.size(); i++)
			if (cookies[i]._id == dest)
				break ;
		if (i == cookies.size())
			break ; 
		dest.clear();
	}
	close(file);
	return dest;
}

//Others functions---------------------------------------------------------

static std::string extractCookieValue(std::string &cookieLine, std::string key)
{
	size_t pos;
	if ((pos = cookieLine.find(key)) == std::string::npos)
		return "";
	pos += key.size();
	size_t end;
	if ((end = cookieLine.find(';', pos)) == std::string::npos)
		end = cookieLine.size();
	return cookieLine.substr(pos, end - pos);
}

static void ModifyCookie(std::vector<char> &fileContent, std::string cookieTheme, std::string target, std::string targetHtml)
{
	std::string tmpContentFile(fileContent.begin(), fileContent.end());
	size_t pos;

	cookieTheme = extractCookieValue(cookieTheme, target);
	if (cookieTheme.empty())
		return ;
	if ((pos = tmpContentFile.find(targetHtml)) != std::string::npos)
	{
		pos += 12;
		size_t end;
		if ((end = tmpContentFile.find('\"', pos)) != std::string::npos)
		{
			if (end - pos <= 30 && tmpContentFile.substr(pos, end - pos) != cookieTheme)
			{
				fileContent.erase(fileContent.begin() + pos, fileContent.begin() + end);
				fileContent.insert(fileContent.begin() + pos, cookieTheme.begin(), cookieTheme.end());
			}
		}
	}
	else if ((pos = tmpContentFile.find("<html")) != std::string::npos)
	{
		pos += 5;
		std::string theme = ' ' + targetHtml + cookieTheme + "\"";
		fileContent.insert(fileContent.begin() + pos, theme.begin(), theme.end());
	}
}

void modifyFile(std::vector<char> &fileContent, const HttpRequest &req)
{
	std::string cookieTheme;

	if (req.header.find("Cookie") == req.header.end())
		return ;
	cookieTheme = req.header.find("Cookie")->second;
	ModifyCookie(fileContent, cookieTheme, "theme=", "data-theme=\"");
	ModifyCookie(fileContent, cookieTheme, "fontsize=", "data-fontsize=\"");
}

static void registerUser(HttpRequest &req, Server &server)
{
	PersonalInfos newInfo;
	std::map<std::string, std::string>::iterator it;

	if ((it = req.body.find("username")) != req.body.end())
		newInfo.setUsername(it->second);
	if ((it = req.body.find("password")) != req.body.end())
		newInfo.setPassword(it->second);
	if ((it = req.body.find("name")) != req.body.end())
		newInfo.setName(it->second);
	if ((it = req.body.find("second_name")) != req.body.end())
		newInfo.setSecondName(it->second);
	if (server.addAccounts(newInfo))
	{
		req.statusCode = 201; //User created
		req.jsonResponse = "{\n\"success\": true,\n\"username\": \""
			+ newInfo.getUsername() + "\",\n\"message\": \"Account sucessfully created !\"\n}";
	}
	else
	{
		req.statusCode = 409; //conflict user already exist
		req.jsonResponse = "{\n\"success\": false,\n\"username\": \""
			+ newInfo.getUsername() + "\",\n\"message\": \"Account already exist !\"\n}";
	}
}

static void logInUser(HttpRequest &req, Cookies &cookie, Server &server)
{
	std::string										username, password;
	int												index = 0;
	std::map<std::string, std::string>::iterator	it;

	if ((it = req.body.find("username")) != req.body.end())
		username = it->second;
	if ((it = req.body.find("password")) != req.body.end())
		password = it->second;

	if (username.empty() || password.empty())
	{
		req.statusCode = 400;
		req.jsonResponse = "{\n\"success\": false,\n\"message\": \"Bad request !\"\n}";
		throw std::runtime_error("Error 400: Bad request");
	}

	if ((index = server.isValidUser(username, password)) < 0)
	{
		req.statusCode = 401;
		req.jsonResponse = "{\n\"success\": false,\n\"message\": \"Unvalid username or password !\"\n}";
		throw std::runtime_error("Error 401: Unvalid identifiants");
	}
	req.statusCode = 201;
	req.jsonResponse = "{\n\"success\": true,\n\"username\": \""
			+ server.getAccounts()[index].getUsername() + "\",\n\"message\": \"Sucessfully logged in !\"\n}";
	cookie.setAuth(Cookies::genCookieId(server.getCookies(), 25));
	server.addAccountIdToInfos(cookie.getAuth(), server.getAccounts()[index]);
	cookie.setModified(1);
	if (cookie.getOutputData().size() > 1)
		cookie.delOutputData(1);
	cookie.addOutputData("auth_token=" + cookie.getAuth() + "; Path=/; Max-Age=300");
}

static void logOutUser(HttpRequest &req, Cookies &cookie, Server &server)
{
	std::string	username, password;

	if (!cookie.getAuth().empty())
		server.delAccountIdToInfos(cookie.getAuth());
	cookie.setModified(1);
	cookie.delOutputData(1);
	cookie.addOutputData("auth_token=" + cookie.getAuth() + "; Path=/; Max-Age=0");
	cookie.setAuth("");
	req.jsonResponse = "{\n\"success\": true,\n\"message\": \"Sucessfully logged out !\"\n}";
	req.statusCode = 200;
}

static void	fillData(Server &server, std::vector<Cookies> &cookies, std::string &cookieLine, HttpRequest &req)
{
	std::string str = extractCookieValue(cookieLine, "id=");
	size_t i;

	for (i = 0; i < cookies.size(); i++)
	{
		if (cookies[i].getId() == str)
			break ;
	}
	if (i == cookies.size())
		return ;
	if (extractCookieValue(cookieLine, "auth_token=") == "" && !cookies[i].getAuth().empty())
	{
		logOutUser(req, cookies[i], server);
		server.setModified(i);
	}
	else if (req.method == "POST")
	{
		if (req.url == "/register")
			registerUser(req, server);
		else if (req.url == "/login")
		{
			logInUser(req, cookies[i], server);
			server.setModified(i);
		}
		else if (req.url == "/logout")
		{
			logOutUser(req, cookies[i], server);
			server.setModified(i);
		}
	}
	else if (req.method == "GET" && req.url == "/me")
	{
		std::map<std::string, PersonalInfos> map = server.getAccountIdToInfos();
		PersonalInfos info = map[cookies[i].getAuth()];
		req.jsonResponse =
			"{"
			"\"success\": true, "
			"\"username\": \"" + info.getUsername() + "\", "
			"\"name\": \"" + info.getName() + "\", "
			"\"second_name\": \"" + info.getSecondName() + "\""
			"}";
	}
}

static void createNewSession(Server &server, std::string oldId, std::string oldAuthToken)
{
	Cookies newCookie;

	if (!oldId.empty())
		newCookie.setPrevId(oldId);
	if (!oldAuthToken.empty())
	{
		newCookie.setPrevAuthToken(oldAuthToken);
	}
	newCookie.setId(Cookies::genCookieId(server.getCookies(), 20));
	newCookie.setModified(0);
	newCookie.addOutputData("id=" + newCookie.getId() + "; Path=/; HttpOnly");
	server.addCookies(newCookie);
	server.setModified(server.getCookies().size() - 1);
}

static int verifyCookieId(std::vector<Cookies> cookies, std::string &str)
{
	for (size_t i = 0; i < cookies.size(); i++)
	{
		if (cookies[i].getId() == str)
			return 0;
	}
	return 1;
}

void	manageCookies(Server &server, HttpRequest &request)
{
	try
	{
		std::string idVal, idAuthToken;
		std::map<std::string, std::string>::iterator it = request.header.find("Cookie");

		if (it != request.header.end())
		{
			idVal = extractCookieValue(it->second, "id=");
			idAuthToken = extractCookieValue(it->second, "auth_token=");
		}
		if (it == request.header.end() || idVal.empty())
			return createNewSession(server, "", "");
		if ((!idVal.empty() && verifyCookieId(server.getCookies(), idVal)))
			return createNewSession(server, idVal, idAuthToken);
		fillData(server, server.getCookies(), it->second, request);
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
	}
}

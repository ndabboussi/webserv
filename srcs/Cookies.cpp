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
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Cookies::Cookies(void): _modified(-1){}

Cookies::Cookies(Cookies const &src): _id(src._id), _prevId(src._prevId), _authToken(src._authToken), _modified(src._modified), _outputData(src._outputData)
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

void Cookies::addOutputData(std::string value)
{
	this->_outputData.push_back(value);
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

void modifyFile(std::vector<char> &fileContent, const HttpRequest &req)
{
	std::string tmpContentFile(fileContent.begin(), fileContent.end());
	size_t pos;
	std::string cookieTheme;

	if (req.header.find("Cookie") == req.header.end())
		return ;
	cookieTheme = req.header.find("Cookie")->second;
	cookieTheme = extractCookieValue(cookieTheme, "theme=");
	if (cookieTheme.empty())
		return ;
	if ((pos = tmpContentFile.find("data-theme=\"")) != std::string::npos)
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
		std::string theme = " data-theme=\"" + cookieTheme + "\"";
		fileContent.insert(fileContent.begin() + pos, theme.begin(), theme.end());
	}
}

static void	fillData(Server &server, std::vector<Cookies> &cookies, std::string &cookieLine)
{
	std::string str = extractCookieValue(cookieLine, "id");
	for (size_t i = 0; i < cookies.size(); i++)
	{
		if (cookies[i].getId() == str)
		{
			if (!extractCookieValue(cookieLine, "auth_token").empty() && cookies[i].getAuth().empty())
			{
				cookies[i].setAuth(Cookies::genCookieId(cookies, 25));
				cookies[i].setModified(1);
				server.setModified(i);
				std::string path = "/images"; //temporary
				cookies[i].addOutputData("id=" + cookies[i].getId() + "; " + "auth_token=" + cookies[i].getAuth() + "; Path=" + path + "; HttpOnly");
			}
		}
	}
}

static void createNewSession(Server &server, std::string oldId)
{
	Cookies newCookie;
	if (!oldId.empty())
		newCookie.setPrevId(oldId);
	newCookie.setId(Cookies::genCookieId(server.getCookies(), 20));
	newCookie.setModified(0);
	newCookie.addOutputData("id=" + newCookie.getId() + "; Path=/; HttpOnly");
	server.addCookies(newCookie);
	server.setModified(server.getCookies().size() - 1);
	return ;
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
		std::string idVal;
		std::map<std::string, std::string>::iterator it = request.header.find("Cookie");

		if (it != request.header.end())
			idVal = extractCookieValue(it->second, "id=");
		if (it == request.header.end() || idVal.empty())
			return createNewSession(server, "");
		if ((!idVal.empty() && verifyCookieId(server.getCookies(), idVal)))
			return createNewSession(server, idVal);
		fillData(server, server.getCookies(), it->second);
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
	}
}

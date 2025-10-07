# include "Server.hpp"

//Operators----------------------------------------------------------------

Cookies &Cookies::operator=(Cookies const &src)
{
    if (this != &src)
    {
    	this->_id = src._id;
		this->_authToken = src._authToken;
		this->_modified = src._modified;
		this->_outputData = src._outputData;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Cookies::Cookies(void): _modified(-1){}

Cookies::Cookies(Cookies const &src): _id(src._id), _authToken(src._authToken), _modified(src._modified), _outputData(src._outputData)
{}

Cookies::~Cookies(void)
{}

//GETTERS------------------------------------------------------------------

std::string	Cookies::getId(void) const
{
	return this->_id;
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
				rd[i] = (rd[i] % 92) + 32;
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
	if ((end = cookieLine.find(';')) == std::string::npos)
		end = cookieLine.size() - 1;
	return cookieLine.substr(pos, end - pos);
}

static void	fillData(std::vector<Cookies> cookies, std::string &cookieLine)
{
	for (size_t i = 0; i < cookies.size(); i++)
	{
		if (cookies[i].getId() == extractCookieValue(cookieLine, "id"))
		{
			if (!extractCookieValue(cookieLine, "auth_token").empty() && cookies[i].getAuth().empty())
			{
				cookies[i].setAuth(Cookies::genCookieId(cookies, 25));
				cookies[i].setModified(1);
				std::string path = "/images"; //temporary
				cookies[i].addOutputData("id=" + cookies[i].getId() + "; " + "auth_token=" + cookies[i].getAuth() + "; Path=" + path + "; HttpOnly");
			}
		}
	}
	
}

static void createNewSession(Server &server)
{
	Cookies newCookie;
	newCookie.setId(Cookies::genCookieId(server.getCookies(), 20));
	newCookie.setModified(0);
	newCookie.addOutputData("id=" + newCookie.getId() + "; Path=/; HttpOnly");
	server.addCookies(newCookie);
	return ;
}

void	manageCookies(Server &server, HttpRequest &request)
{
	try
	{
		std::map<std::string, std::string>::iterator it = request.header.find("Cookie");
		if (it == request.header.end() || extractCookieValue(it->second, "id").empty())
			return createNewSession(server);
		fillData(server.getCookies(), it->second);
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
	}
}

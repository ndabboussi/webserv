# include "Server.hpp"

//Operators----------------------------------------------------------------

Cookies &Cookies::operator=(Cookies const &src)
{
    if (this != &src)
    {
    	this->_id = src._id;
		this->_data = src._data;
		this->_modified = src._modified;
		this->_outputData = src._outputData;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

Cookies::Cookies(void){}

Cookies::Cookies(Cookies const &src): _id(src._id), _data(src._data), _modified(src._modified), _outputData(src._outputData)
{}

Cookies::~Cookies(void)
{}

//GETTERS------------------------------------------------------------------

std::string	Cookies::getId(void) const
{
	return this->_id;
}

std::map<std::string, std::map<std::string, std::string> >	Cookies::getData(void) const
{
	return this->_data;
}

int Cookies::getModified(void) const
{
	return this->_modified;
}

std::map<std::string, std::string> Cookies::getOutputData(void) const
{
	return this->_outputData;
}

//SETTERS------------------------------------------------------------------

void Cookies::addData(std::string path, std::string key, std::string value)
{
	std::map<std::string, std::map<std::string, std::string> >::iterator it = this->_data.find(path);
	if (it == this->_data.end())
	{
		std::map<std::string,std::string> newMap;
		newMap.insert(std::make_pair(key, value));
		this->_data.insert(std::make_pair(path, newMap));
		return ;
	}
	std::map<std::string, std::string> &map = it->second;
	if (map.find(key) == map.end())
		map.insert(std::make_pair(key, value));
	else
		map[key] = value;
}

void Cookies::setModified(int index)
{
	this->_modified = index;
}

void Cookies::addOutputData(std::string key, std::string value)
{
	if (this->_outputData.find(key) == this->_outputData.end())
		this->_outputData.insert(std::make_pair(key, value));
	else
		this->_outputData[key] = value;
}

//Member function----------------------------------------------------------


void Cookies::genCookieId(std::vector<Cookies> const &cookies)
{
	int file = open("/dev/urandom", O_RDONLY);
	unsigned char rd[21] = {0};

	if (file < 0)
		throw std::runtime_error("Error: cannot open file");
	while (1)
	{
		if (read(file, rd, 20) < 0)
		{
			close(file);
			throw std::runtime_error("Error: Read function failed");
		}
		for (size_t i = 0; i < 20; i++)
		{
			if (rd[i] > 126)
				rd[i] %= 127;
			if (rd[i] < 32)
				rd[i] += 32;
			this->_id += rd[i];
		}
		size_t i;
		for (i = 0; i < cookies.size(); i++)
			if (cookies[i]._id == this->_id)
				break ;
		if (i == cookies.size())
			break ; 
		this->_id.clear();
	}
	close(file);
}

//Others functions---------------------------------------------------------

static void	fillData(std::vector<Cookies> &cookies, HttpRequest &request)
{
	;
}

void	manageCookies(Server &server, HttpRequest &request)
{
	try
	{
		std::map<std::string, std::string>::iterator it = request.header.find("Cookie");
		if (it == request.header.end())
		{
			Cookies newCookie;
			newCookie.genCookieId(server.getCookies());
			newCookie.setModified(0);
			newCookie.addOutputData("/", "id=" + newCookie.getId() + "; HttpOnly");
			server.addCookies(newCookie);
			return ;
		}
		//fillData(newCookie, request);
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
	}
}

#include "Location.hpp"

//Operators----------------------------------------------------------------

Location &Location::operator=(Location const &src)
{
	if (this == &src)
		return (*this);
	this->_locations = src._locations;
	this->_data = src._data;
	this->_methods = src._methods;
	this->_path = src._path;
	this->_autoIndex = src._autoIndex;
	return (*this);
}

//Constructor/Destructors--------------------------------------------------

Location::Location(void): _path(""), _autoIndex(false), _locations(), _data(), _methods(0){};

Location::Location(Location const &src)
{
	*this = src;
	return ;
}

Location::~Location(void){}

//Member functions---------------------------------------------------------

std::vector<Location> const Location::getLocations() const
{
	return (this->_locations);
}

std::map<std::string, std::string> const Location::getData() const
{
	return (this->_data);
}

uint8_t	Location::getMethods() const
{
	return (this->_methods);
}

std::string Location::getPath() const
{
	return (this->_path);
}

bool Location::getAutoIndex(void) const
{
	return this->_autoIndex;
}

void Location::addLocations(Location newLoc)
{
	this->_locations.push_back(newLoc);
	return ;
}

void Location::cpyData(std::map<std::string, std::string> data)
{
	this->_data = data;
	return ;
}

void Location::addData(std::string key, std::string value)
{
	if (this->_data.find(key) == this->_data.end())
		this->_data.insert(std::make_pair(key, value));
	else
		this->_data[key] = value;//remplace la key si elle existe deja
	//this->_data.insert(std::make_pair(key, value));//ne remplace pas la key si elle existe deja
	return ;
}

void Location::setMethods(uint8_t methods)
{
	this->_methods = methods;
	return ;
}

void Location::setAutoIndex(bool flag)
{
	this->_autoIndex = flag;
	return ;
}

void Location::setPath(std::string path)
{
	this->_path = path;
	return ;
}
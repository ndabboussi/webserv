#include "Location.hpp"

//Operators----------------------------------------------------------------

Location &Location::operator=(Location const &src)
{
	if (this == &src)
		return (*this);
	this->_locations = src._locations;
	this->_data = src._data;
	this->_alias = src._alias;
	this->_methods = src._methods;
	return (*this);
}

//Constructor/Destructors--------------------------------------------------

Location::Location(void): _locations(), _data(), _alias(false), _methods(7){};

Location::Location(Location const &src)
{
	*this = src;
	return ;
};

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

// bool const Location::getAlias() const
bool	Location::getAlias() const
{
	return (this->_alias);
}

// uint8_t	const Location::getMethods() const
uint8_t	Location::getMethods() const
{
	return (this->_methods);
}

void Location::addLocations(Location newLoc)
{
	this->_locations.push_back(newLoc);
	return ;
}

void Location::addData(std::string key, std::string value)
{
	// this->_data.insert(key, value);
	this->_data[key] = value;//remplace la key si elle existe deja
	//this->_data.insert(std::make_pair(key, value));//ne remplace pas la key si elle existe deja
	return ;
}

void Location::setAlias(bool alias)
{
	this->_alias = alias;
	return ;
}

void Location::setMethods(uint8_t methods)
{
	this->_methods = methods;
	return ;
}
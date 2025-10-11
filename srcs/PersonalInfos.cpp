#include "PersonalInfos.hpp"

//Operators----------------------------------------------------------------

PersonalInfos &PersonalInfos::operator=(PersonalInfos const &src)
{
    if (this != &src)
    {
		this->_username = src._username;
		this->_password = src._password;
		this->_name = src._name;
		this->_secondName = src._secondName;
    }
    return *this;
}

//Constructor/Destructors--------------------------------------------------

PersonalInfos::PersonalInfos(void)
{}

PersonalInfos::PersonalInfos(PersonalInfos const &src) : _username(src._username), _password(src._password),
			_name(src._name), _secondName(src._secondName)
{}

PersonalInfos::~PersonalInfos(void)
{}

//GETTERS ---------------------------------------------------------

void	PersonalInfos::setUsername(std::string value)
{
	this->_username = value;
}

void	PersonalInfos::setPassword(std::string value)
{
	this->_password = value;
}

void	PersonalInfos::setName(std::string value)
{
	this->_name = value;
}

void	PersonalInfos::setSecondName(std::string value)
{
	this->_secondName = value;
}

//SETTERS ---------------------------------------------------------


std::string	PersonalInfos::getUsername() const
{
	return this->_username;
}

std::string	PersonalInfos::getPassword() const
{
	return this->_password;
}

std::string	PersonalInfos::getName() const
{
	return this->_name;
}

std::string	PersonalInfos::getSecondName() const
{
	return this->_secondName;
}
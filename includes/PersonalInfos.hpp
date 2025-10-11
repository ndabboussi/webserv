#ifndef PERSONALINFOS_HPP

# define PERSONALINFOS_HPP

# include "Server.hpp"

class	PersonalInfos
{
	private:
		std::string	_username;
		std::string	_password;
		std::string	_name;
		std::string	_secondName;


	public:
		PersonalInfos(void);
		PersonalInfos(PersonalInfos const &src);
		PersonalInfos &operator=(PersonalInfos const &src);
		~PersonalInfos(void);

	public:
		void		setUsername(std::string value);
		void		setPassword(std::string value);
		void		setName(std::string value);
		void		setSecondName(std::string value);

		std::string	getUsername() const;
		std::string	getPassword() const;
		std::string	getName() const;
		std::string	getSecondName() const;
};

#endif
#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include "AMessage.hpp"

class Request : public AMessage {
	public:
		Request();										//Default constructor
		~Request();										//Destructor
		Request(const Request &other);				//Copy constructor
		Request &operator=(const Request &other);	//Copy operator

		bool	processMsg();

		const std::string&	getMethode() const;
		const std::string&	getUri() const;
		const std::string&	getVersion() const;

	private:
		void	processEntry();
		void	processHeader();
		void	processBody();

		std::string	_methode;
		std::string	_uri;
		std::string	_version;
};

std::ostream&	operator<<(std::ostream& out, const Request& el);

#endif

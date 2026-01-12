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

	private:
		bool	processEntry();
		bool	processHeader();
		bool	processBody();

		std::string	_methode;
		std::string	_uri;
		std::string	_version;
};
#endif

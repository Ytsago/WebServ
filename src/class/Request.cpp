#include "Request.hpp"

Request::Request() {
	std::cout << "Default constructor called" << std::endl;
}

Request::Request(const Request &other) : /* copy field */{
	std::cout << "Copy constructor called" << std::endl;
}

Request	&Request::operator=(const Request &other) {
	std::cout << "Copy assignment operator called" << std::endl;
	if (this != &other)
		//copy field
	return (*this);
}

Request::~Request() {
	std::cout << "Destructor called" << std::endl;
}

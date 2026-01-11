#include "Respond.hpp"

Respond::Respond() {
	std::cout << "Default constructor called" << std::endl;
}

Respond::Respond(const Respond &other) : /* copy field */{
	std::cout << "Copy constructor called" << std::endl;
}

Respond	&Respond::operator=(const Respond &other) {
	std::cout << "Copy assignment operator called" << std::endl;
	if (this != &other)
		//copy field
	return (*this);
}

Respond::~Respond() {
	std::cout << "Destructor called" << std::endl;
}

#include "AMessage.hpp"

AMessage::AMessage() {
	std::cout << "Default constructor called" << std::endl;
}

AMessage::AMessage(const AMessage &other) : /* copy field */{
	std::cout << "Copy constructor called" << std::endl;
}

AMessage	&AMessage::operator=(const AMessage &other) {
	std::cout << "Copy assignment operator called" << std::endl;
	if (this != &other)
		//copy field
	return (*this);
}

AMessage::~AMessage() {
	std::cout << "Destructor called" << std::endl;
}

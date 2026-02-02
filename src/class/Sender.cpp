#include "Sender.hpp"

Sender::Sender() {
}

Sender::Sender(const Sender &other) {
	(void) other;
}

Sender	&Sender::operator=(const Sender &other) {
	(void) other;
	return (*this);
}

Sender&	Sender::getInstance() {
	static Sender	instance;

	return instance;
}

Sender::~Sender() {
}

#include "ANetContainer.hpp"
#include <unistd.h>

ANetContainer::ANetContainer() : socket(-1), logs(std::cout), errLogs(std::cerr) {
}

ANetContainer::ANetContainer(std::ostream& logs, std::ostream& errLogs) : socket(-1), logs(logs), errLogs(errLogs) {
}

ANetContainer::ANetContainer(const ANetContainer &other) : socket(other.socket), logs(other.logs), errLogs(other.errLogs) {
}

ANetContainer	&ANetContainer::operator=(const ANetContainer &other) {
	if (this != &other) {
		socket = other.socket;
	}
	return (*this);
}

void	ANetContainer::setSocket(const int& socket) {this->socket = socket;}
int		ANetContainer::getSocket() const {return socket;}

ANetContainer::~ANetContainer() {
	close(socket);
}

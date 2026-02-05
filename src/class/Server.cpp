#include "Server.hpp"

Server::Server() : ANetContainer() {
}

Server::Server(std::ostream& logs, std::ostream& errLogs) : ANetContainer(logs, errLogs) {
}

Server::Server(const Server &other) : ANetContainer(other) {
}

int	Server::get_type() const {
	return (SERVER);
}

Server	&Server::operator=(const Server &other) {
	if (this != &other)
		this->::ANetContainer::operator=(other);
	return (*this);
}

Server::~Server() {
}

#include "Client.hpp"
#include "ANetContainer.hpp"

Client::Client() : ANetContainer(), index(0) {
}

Client::Client(std::ostream& logs, std::ostream& errLogs) : ANetContainer(logs, errLogs), index(0) {
}

Client::Client(const Client &other) : ANetContainer(other), request(other.request), response(other.response), index(other.index) {}

bool	Client::isClient() const {
	return true;
}

Client	&Client::operator=(const Client &other) {
	if (this != &other) {
		this->::ANetContainer::operator=(other);
		this->index = other.index;
		this->request = other.request;
		this->response = other.response;
	}
	return (*this);
}

Request&	Client::getRequest() {return request;}
const size_t&	Client::getIndex() const {return index;}
void	Client::setIndex(size_t newIndex) {index = newIndex;}

Client::~Client() {
}

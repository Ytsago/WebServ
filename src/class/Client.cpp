#include "Client.hpp"
#include "ANetContainer.hpp"

Client::Client() : ANetContainer(), index(0), _response(NULL) {
}

Client::Client(std::ostream& logs, std::ostream& errLogs) : ANetContainer(logs, errLogs), index(0), _response(NULL) {
}

Client::Client(const Client &other) : ANetContainer(other), index(other.index), _response(other._response) {}

int	Client::get_type() const {
	return (CLIENT);
}
HttpParser&	Client::getParser() {return (this->parser);}


Client	&Client::operator=(const Client &other) {
	if (this != &other) {
		this->::ANetContainer::operator=(other);
		this->index = other.index;
		// this->response = other.response;
	}
	return (*this);
}

const size_t&	Client::getIndex() const {return index;}
Response	*Client::getResponse() {return _response;}
void	Client::setIndex(size_t newIndex) {index = newIndex;}
void	Client::setResponse(Response &response) {
	this->_response = &response;
}


Client::~Client() {
}

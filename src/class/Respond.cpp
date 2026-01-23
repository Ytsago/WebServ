#include "Respond.hpp"

static const char PAT[4] = {'\r', '\n', '\r', '\n'};

Respond::Respond() : AMessage() {
}

Respond::Respond(const Respond &other) : AMessage(other) {
}

Respond	&Respond::operator=(const Respond &other) {
	this->::AMessage::operator=(other);
	return (*this);
}

void	Respond::buildMsg() {
	byteVector	msg(_entryLine);

	msg.insert(msg.end(), PAT, PAT + 4);
	for (std::map<std::string, std::string>::const_iterator it = _headerField.begin(); it != _headerField.end(); it++) {
		msg.insert(msg.end(), it->first.begin(), it->first.end() -1);
		msg.push_back(':');
		msg.insert(msg.end(), it->second.begin(), it->second.end() -1);
		msg.insert(msg.end(), PAT, PAT + 4);
	}
	msg.insert(msg.end(), PAT, PAT + 4);
	if (this->checkFlag(BODY))
		msg.insert(msg.end(), _body.begin(), _body.end() -1);
	setRaw(msg);
}

Respond::~Respond() {
}

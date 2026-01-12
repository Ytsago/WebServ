#include "Request.hpp"
#include <cstring>

static const byte PATTERN[4] = {'\r', '\n', '\r', '\n'};

Request::Request() : AMessage() {
}

Request::Request(const Request &other) : AMessage(other) {
}

Request	&Request::operator=(const Request &other) {
	this->AMessage::operator=(other);
	return *this;
}

//[TODO] Continue parsing here
bool	Request::processEntry() {
	byteVector&	raw = getRaw();
	std::string*	info[3] = {&_methode, &_uri, &_version};

	for (size_t i = 0; i + 3 < raw.size(); i++) {
		if (std::memcmp(&raw[i], PATTERN, 4) == 0) {
			setFlag(ENTRY);
			return 0;
		}
	}
}

bool	Request::processHeader() {
	
}

bool	Request::processBody() {

}

bool	Request::processMsg() {
	byte f = getFlag();

	if (!(f & ENTRY)) {

	}
	if (!(f & HEADER)) {
		
	}
	if (!(f & BODY)) {

	}
}

Request::~Request() {
}

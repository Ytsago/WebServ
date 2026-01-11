#include "AMessage.hpp"

AMessage::AMessage() : flags(0) {
}

AMessage::AMessage(const AMessage &other) : _entryLine(other._entryLine), _body(other._body), _headerField(other._headerField), flags(0){
}

std::vector<byte>	AMessage::buildMsg() const {
	std::vector<byte>	msg(_entryLine);

	msg.insert(msg.end(), _body.begin(), _body.end());
}

inline void	AMessage::setFlag(byte flag) {
	flags |= flag;
}

inline bool	AMessage::getFlag(byte flag) const {
	return (flags & flag) > 0;
}

inline void	AMessage::clear() {
	*this = AMessage();
}

AMessage	&AMessage::operator=(const AMessage &other) {
	std::cout << "Copy assignment operator called" << std::endl;
	if (this != &other) {
		this->_entryLine = other._entryLine;
		this->_body = other._body;
		this->_headerField = other._headerField;
		this->flags = other.flags;
	}
	return (*this);
}

AMessage::~AMessage() {
}

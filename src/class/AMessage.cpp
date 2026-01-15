#include "AMessage.hpp"


AMessage::AMessage() : flags(0) {
}

AMessage::AMessage(const AMessage &other) : _entryLine(other._entryLine), _body(other._body), _headerField(other._headerField), flags(0){
}

// byteVector	AMessage::buildMsg() const {
// 	byteVector	msg(_entryLine);
//
// 	msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	for (std::map<std::string, std::string>::const_iterator it = _headerField.begin(); it != _headerField.end(); it++) {
// 		msg.insert(msg.end(), it->first.begin(), it->first.end() -1);
// 		msg.push_back(':');
// 		msg.insert(msg.end(), it->second.begin(), it->second.end() -1);
// 		msg.insert(msg.end(), PAT, PAT + 4);
// 	}
// 	msg.insert(msg.end(), PAT, PAT + 4);
// 	msg.insert(msg.end(), _body.begin(), _body.end() -1);
// 	return msg;
// }

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

void	AMessage::setFlag(byte flag) {flags |= flag;}
const byte&	AMessage::getFlag() const {return flags;}
bool	AMessage::checkFlag(byte flag) const {return (flags & flag) > 0;}
bool	AMessage::eof() const {return (flags & EOF) > 0;}
bool	AMessage::fail() const {return (flags & FAIL) > 0;}
void	AMessage::clear() {*this = AMessage();}

byteVector&	AMessage::getRaw() {return _raw;}
void	AMessage::setRaw(byteVector data) {_raw = data;}

AMessage::~AMessage() {
}

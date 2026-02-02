#include "Request.hpp"
#include <cstring>
#include <sstream>
#include <algorithm>

// static const byte PATTERN[4] = {'\r', '\n', '\r', '\n'};

Request::Request() : AMessage() {
}

Request::Request(const Request &other) : AMessage(other) {
}

Request	&Request::operator=(const Request &other) {
	this->AMessage::operator=(other);
	return *this;
}
/*
//[TODO] Continue parsing here
void	Request::processEntry() {
	byteVector&	raw = getRaw();
	std::string*	info[3] = {&_method, &_uri, &_version};
	size_t	j = 0;

	for (size_t i = 0; i + 3 < raw.size(); i++) {
		if (std::memcmp(&raw[i], PATTERN, 2) == 0) {
			setFlag(FLAG_ENTRY);
			if (info[0]->empty() || info[1]->empty() || info[2]->empty())
				// setFlag(FLAG_FAIL);
			return ;
		}
		if (raw[i] == ' ')
			j++;
		if (j > 2) {
			// setFlag(FLAG_FAIL);
			return ;
		}
		else
			info[j] += raw[i];
	}
}
*/

void	Request::processEntry() {
	byteVector	&raw = this->getRaw();
	byteVector::iterator	endLine;

	endLine = std::find(raw.begin(), raw.end(), '\r');
	if (endLine == raw.end() || endLine +1 == raw.end()) {
		return ;
	}
	if (endLine + 1 != raw.end() && *(endLine + 1) != '\n') {
		return ;
	}

	//TODO Add security check for bad input. Also, whitespace may be consecutive in final version
	//Check that i does not exceed 2
	std::stringstream	iss(std::string(raw.begin(), endLine));
	iss >> _method >> _uri >> _version;
	this->setFlag(FLAG_ENTRY);
	raw.erase(raw.begin(), endLine + 2);
}

void	Request::processHeader() {
	byteVector	&raw = this->getRaw();
	byteVector::iterator	endLine;

	endLine = std::find(raw.begin(), raw.end(), '\r');
	if (endLine == raw.end() || endLine +1 == raw.end()) {
		// this->setFlag(FLAG_EOF);
		return ;
	}

	if (endLine + 1 != raw.end() && *(endLine + 1) != '\n') {
		this->setFlag(FLAG_FAIL);
		return ;
	}

	if (endLine == raw.begin()) {
		raw.erase(raw.begin(), raw.begin() + 2);
		this->setFlag(FLAG_HEADER);
		return ;
	}

	std::string	field(raw.begin(), endLine);
	size_t	commaPos = field.find(':');
	
	if (commaPos == field.npos) {
		setFlag(FLAG_FAIL);
		return ;
	}

	this->_headerField[field.substr(0, commaPos)] = field.substr(commaPos + 1, field.size());
	raw.erase(raw.begin(), endLine + 2);
}

//TODO Add check using content lenght ex :
// Content-Lenght: 200, body + raw > 200 -> ERROR
void	Request::processBody() {
	byteVector&	raw = getRaw();

	this->_body.insert(_body.end(), raw.begin(), raw.end());
	raw.erase(raw.begin(), raw.end());
	return ;
}

bool	Request::processMsg() {
	const byte& flag = this->getFlag();

	while (!(checkFlag(FLAG_EOF | FLAG_FAIL))) {
		size_t prevSize = getRaw().size();
		if (this->fail())
			return 1;
		if ((flag & FLAG_ENTRY) == 0)
			this->processEntry();
		else if ((flag & FLAG_HEADER) == 0)
			this->processHeader();
		else if ((flag & FLAG_BODY) == 0) {
			this->processBody();
		}

		if (_method == "GET" && (flag & FLAG_HEADER) > 0) {
			setFlag(FLAG_EOF);
			break ;
		}
		if (prevSize == getRaw().size())
			break ;
	}
	//TODO remove this line (it's for testing purpose)
	return 0;
}

std::string	Request::get_method() const {return _method;}
std::string	Request::get_uri() const {return _uri;}
std::string	Request::get_version() const {return _version;}

Request::~Request() {
}

std::ostream&	operator<<(std::ostream& out, const Request& el) {
	out << "Entry Line: \n";
	out << el.get_method() << " " << el.get_uri() << " " << el.get_version() << "\n";

	out << "\nHeader fields: \n";
	for (headerMap::const_iterator it = el.getHeader().begin(); it != el.getHeader().end(); it++) {
		out << it->first << ": " << it->second << "\n";
	}

	out << "\nBody: \n";
	out << std::string(el.getBody().begin(), el.getBody().end());
	return out;
}

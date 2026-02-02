#include "HttpParser.hpp"
#include <algorithm>
#include "utils.hpp"

HttpParser::HttpParser() : m_state(REQUEST_LINE), m_cursor(0) {
	m_readBuf.reserve(8192);
	m_header.reserve(20);
}

std::string	HttpParser::getHeader(const std::string& key) const {
	for (size_t i = 0; i < m_header.size(); i++) {
		if (m_header[i].first == key)
			return m_header[i].second;
	}
	return "";
}

bool	HttpParser::isComplete() const {return m_state == COMPLETE;}

void	HttpParser::consume(const char* data, size_t len) {
	if (len + m_readBuf.size() > 8192 && m_state != BODY)
		throw HttpRequestParsingException(REQUEST_HEADER_FIELD_TOO_LARGE);
	m_readBuf.insert(m_readBuf.end(), data, data + len);

	bool	again = true;
	while (again) {
		switch (m_state) {
			case REQUEST_LINE: again = parseRequestLine(); break;
			case HEADER: again = parseHeader(); break;
			case BODY: again = parseBody(); break;
			case COMPLETE: again = false; break;	//TODO add a check for Host
			case CHUNKEDBODY: throw HttpRequestParsingException(NOT_IMPLEMENTED);
			default: again = false; break;
		}
	}
}

bool	HttpParser::parseRequestLine() {
	std::vector<char>::iterator	itStart = m_readBuf.begin() + m_cursor;
	std::vector<char>::iterator	itEndLine = std::search(itStart, m_readBuf.end(), "\r\n", ("\r\n") +2);

	if (itEndLine == m_readBuf.end()) {
		if (m_readBuf.size() - m_cursor > 2048)
			throw HttpRequestParsingException(URI_TOO_LONG);
		return false;
	}
	
	if (std::distance(itStart, itEndLine) > 2048)
		throw HttpRequestParsingException(URI_TOO_LONG);

	std::vector<char>::iterator itFirstSpace, itSecondSpace;
	itFirstSpace = std::find(itStart, itEndLine, ' ');
	if (itFirstSpace == itEndLine)
		throw HttpRequestParsingException(BAD_REQUEST);
	itSecondSpace = std::find(itFirstSpace + 1, itEndLine, ' ');
	if (itSecondSpace == itEndLine)
		throw HttpRequestParsingException(BAD_REQUEST);

	m_method.assign(itStart, itFirstSpace);
	m_path.assign(itFirstSpace + 1, itSecondSpace);
	std::string	version(itSecondSpace + 1, itEndLine);

	if (m_method.empty() || m_path.empty() || version.empty())
		throw HttpRequestParsingException(BAD_REQUEST);
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		throw HttpRequestParsingException(HTTP_VERSION_NOT_SUPPORTED);

	m_cursor += std::distance(itStart, itEndLine + 2);
	m_state = HEADER;
	return true;
}

HttpParser::MediaType	HttpParser::getMediaType(std::string& media) {
	if (media == "text/plain") return TEXT;
	if (media == "multipart/form-data") return MULTIPART;
	if (media == "application/url_encoded") return APPLICATION;
	throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
}

bool	HttpParser::processHeader() {
	std::string header;
	if (!(header = getHeader("transfer-encoding")).empty()) {
		if (header != "chunked")
			throw HttpRequestParsingException(NOT_IMPLEMENTED);
		m_state = CHUNKEDBODY;
	}
	else if (!(header = getHeader("content-length")).empty()) {
		char*	endptr;
		long	val;
		val = std::strtol(header.c_str(), &endptr, 10);
		if (*endptr != '\0' || val < 0 || val > MAX_BODY_SIZE)
			throw HttpRequestParsingException(BAD_REQUEST);
		m_contentLength = static_cast<size_t>(val);
		if (m_contentLength > 0) {
			m_state = BODY;
			m_body.reserve(m_contentLength);
		}
	}
	else
		return false;
	if (!(header = getHeader("content-type")).empty()) {
		size_t	semiColon = header.find(';');
		std::string	media = header.substr(0, semiColon);
		strLower(media);
		m_type = getMediaType(media);
		switch (media) {
			case MULTIPART: 
				if (semiColon == std::string::npos)
					throw HttpRequestParsingException(BAD_REQUEST);
				size_t	boundIndex = media.find("boundary")
		}
	}
	return true;
}

//TODO add check for host
bool	HttpParser::parseHeader() {
	std::vector<char>::iterator	itStart = m_readBuf.begin() + m_cursor;
	std::vector<char>::iterator	itEndLine = std::search(itStart, m_readBuf.end(), "\r\n", ("\r\n") +2);

	if (itEndLine == m_readBuf.end())
		return false;

	if (itEndLine == itStart) {
		m_cursor += 2;
		if (processHeader())
			m_state = COMPLETE;
		return true;
	}

	std::vector<char>::iterator	itSeparator = std::find(itStart, itEndLine, ':');
	if (itSeparator == itEndLine)
		throw HttpRequestParsingException(BAD_REQUEST);

	std::string	key(itStart, itSeparator);
	if (key.empty())
		throw HttpRequestParsingException(BAD_REQUEST);

	std::string	value(itSeparator + 1, itEndLine);
	size_t	firstSpace = value.find_first_not_of(' ');
	if (firstSpace == std::string::npos)
		value = "";
	else {
		size_t secondSpace = value.find_last_not_of(' ');
		value = value.substr(firstSpace, secondSpace - firstSpace + 1);
	}
	strLower(key);
	if (canBeLowered(key))
		strLower(value);

	std::vector<std::pair<std::string, std::string> >::iterator	it;
	for (it = m_header.begin(); it != m_header.end(); it++)
		if (it->first == key)
			break;
	if (it == m_header.end())
		m_header.push_back(std::pair<std::string, std::string>(key, value));
	else 
		it->second += ", " + value;

	m_cursor += std::distance(itStart, itEndLine + 2);
	return true;
}

bool	HttpParser::parseBody() {
	size_t	bytesNeeded = m_contentLength - m_body.size();
	size_t	available = m_readBuf.size() - m_cursor;

	if (available > 0) {
		size_t	toCopy = (available < bytesNeeded) ? available : bytesNeeded;
		m_body.insert(m_body.end(), m_readBuf.begin() + m_cursor, m_readBuf.begin() + toCopy+ m_cursor);
		m_cursor += toCopy;
	}

	if (m_body.size() == m_contentLength) {
		m_state = COMPLETE;
		return true;
	}

	if (m_cursor >= m_readBuf.size()) {
		m_cursor = 0;
		m_readBuf.clear();
	}
	return false;
}

void	HttpParser::reset() {
	if (m_cursor == m_readBuf.size()) m_readBuf.clear();
	else m_readBuf.erase(m_readBuf.begin(), m_readBuf.begin() + m_cursor);

	m_cursor = 0;
	m_contentLength = 0;
	m_state = REQUEST_LINE;
}

HttpRequest*	HttpParser::generateRequest() {
	HttpRequest*	request = new HttpRequest;

	request->_method.swap(m_method);
	request->_body.swap(m_body);
	request->_header.swap(m_header);
	request->_uri.swap(m_path);
	request->_contentLength = m_contentLength;

	this->reset();
	return request;
}

HttpParser::~HttpParser() {

}

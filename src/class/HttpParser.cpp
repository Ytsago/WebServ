#include "HttpParser.hpp"
#include "StatusCode.hpp"
#include <algorithm>
#include "Logger.hpp"
#include <cstddef>
#include "utils.hpp"

HttpParser::HttpParser() : m_state(REQUEST_LINE), m_type(NONE), m_cursor(0), m_bodySize(0), m_readBuf() {
	m_readBuf.reserve(8192);
	m_headers.reserve(20);
}

std::string	HttpParser::getHeader(const std::string& key) const {return m_headers[key];}

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
		if (m_readBuf.size() - m_cursor > 2048) {
			throw HttpRequestParsingException(URI_TOO_LONG);
		}
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

void	HttpParser::parseMediaType(std::string& rawMedia) {
	size_t	slashPos = rawMedia.find('/');

	if (slashPos == std::string::npos || slashPos == rawMedia.size() -1 || slashPos == 0) {
		Logger::record(ERROR) << "1";
		throw HttpRequestParsingException(BAD_REQUEST);
	}

	std::string	media = rawMedia.substr(0, slashPos);
	m_subtype = rawMedia.substr(slashPos +1);
	trim(media);
	trim(m_subtype);

	if (m_subtype.empty()) {
		Logger::record(ERROR) << "2";
		throw HttpRequestParsingException(BAD_REQUEST);
	}
	if (media == "text") m_type = TEXT;
	else if (media == "application") m_type = APPLICATION;
	else if (media == "multipart") m_type = MULTIPART;
	else throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
}

std::string	extractBoundary(std::string& header) {
	std::string tmp(header);
	strLower(tmp);

	size_t	boundIndex = tmp.find("boundary=");
	if (boundIndex == std::string::npos) return "";

	size_t	len;
	size_t	boundEnd = tmp.find(" \t;", boundIndex +9);
	if (boundEnd == std::string::npos)
		len = std::string::npos;
	else
		len = boundEnd - (boundIndex + 9);

	std::string	boundary = header.substr(boundIndex + 9, len);
	if (boundary.size() >= 2 && boundary[0] == '"' && boundary[boundary.size() -1] == '"')
		boundary = boundary.substr(1, boundary.size() -2);
	return (boundary);
}

void	HttpParser::processHeader() {
	std::string header;
	if (getHeader("host").empty())
		throw BAD_REQUEST;
	if (!(header = getHeader("transfer-encoding")).empty()) {
		if (header != "chunked")
			throw HttpRequestParsingException(NOT_IMPLEMENTED);
		m_state = CHUNKEDBODY;
	}
	else if (!(header = getHeader("content-length")).empty()) {
		char*	endptr;
		long	val;
		val = std::strtol(header.c_str(), &endptr, 10);
		if (*endptr != '\0' || val < 0 || val > MAX_BODY_SIZE) {
			throw HttpRequestParsingException(BAD_REQUEST);
		}
		m_contentLength = static_cast<size_t>(val);
		if (m_contentLength > 0) {
			m_state = BODY;
		}
		else
			m_state = COMPLETE;
	}
	else {
		m_state = COMPLETE;
		return ;
	}
	if (!(header = getHeader("content-type")).empty()) {
		size_t	semiColon = header.find(';');
		std::string	media = header.substr(0, semiColon);
		if (media.empty())
			return ;
		strLower(media); parseMediaType(media);
		switch (m_type) {
			case TEXT:
				if (m_subtype.empty()) {
					// throw HttpRequestParsingException(BAD_REQUEST);
					break;
				}
				break;
			case APPLICATION:
				if (m_subtype == "x-www-form-urlencoded") {
					throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
					break ;
				}
				else if (m_subtype == "octet-stream")
					m_type = TEXT;
				else
					throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
				break ;
			case MULTIPART:
				if (m_subtype != "form-data")
					throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
				break ;
			default:
				throw HttpRequestParsingException(UNSUPORTED_MEDIA_TYPE);
		}
	}
	return ;
}

//TODO add check for host
bool	HttpParser::parseHeader() {
	std::vector<char>::iterator	itStart = m_readBuf.begin() + m_cursor;
	std::vector<char>::iterator	itEndLine = std::search(itStart, m_readBuf.end(), "\r\n", ("\r\n") +2);

	if (itEndLine == m_readBuf.end())
		return false;

	if (itEndLine == itStart) {
		m_cursor += 2;
		processHeader();
		return true;
	}

	std::vector<char>::iterator	itSeparator = std::find(itStart, itEndLine, ':');
	if (itSeparator == itEndLine) {
		Logger::record(ERROR) << "5";
		throw HttpRequestParsingException(BAD_REQUEST);
	}

	std::string	key(itStart, itSeparator);
	if (key.empty()) {
		Logger::record(ERROR) << "6";
		throw HttpRequestParsingException(BAD_REQUEST);
	}

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
	for (it = m_headers.begin(); it != m_headers.end(); it++)
		if (it->first == key)
			break;
	if (it == m_headers.end())
		m_headers.add(key, value);
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
		m_bodySize += toCopy;
		if (m_type == MULTIPART)
			m_state = COMPLETE;
	}

	if (m_bodySize == m_contentLength) {
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
	m_type = NONE;
	m_bodySize = 0;
	m_body = byteVector();
	m_method = std::string();
	m_headers = HeaderMap();
	m_path = std::string();
	m_subtype = std::string();
	m_boundary = std::string();
}

HttpRequest*	HttpParser::generateRequest() {
	HttpRequest*	request = new HttpRequest;

	request->_method.swap(m_method);
	request->_body.swap(m_body);
	request->_header.swap(m_headers);
	request->_uri.swap(m_path);
	request->_contentLength = m_contentLength;

	this->reset();
	return request;
}

HttpParser::~HttpParser() {

}

const char*	HttpParser::HttpRequestParsingException::what() const throw() {
	return g_status_map[e_status].c_str(); 
}


#include <cctype>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

# define MAX_BODY_SIZE 1000000 //Has to depend on the host server

class HttpRequest;

enum	ParserState {REQUEST_LINE, HEADER, BODY, CHUNKEDBODY, COMPLETE};

const std::string	SPECIFIC_KEYS[] = {"transfer-encoding", "connection"}; //ADD here the key that will be lowered

enum	StatusCode {
	OK = 200,
	NOT_MODIFIED = 304,
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHODE_NOT_ALLOWED = 405,
	LENGHT_REQUIRED = 411,
	CONTENT_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	UNSUPORTED_MEDIA_TYPE = 415,
	TOO_MANY_REQUEST = 429,
	REQUEST_HEADER_FIELD_TOO_LARGE = 431,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	HTTP_VERSION_NOT_SUPPORTED = 505
};

void	strLower(std::string& str) {
	std::string::iterator	it = str.begin();

	for (; it != str.end(); it++)
		*it = tolower(*it);
}

bool	canBeLowered(const std::string& key) {
	for (size_t i = 0; i < sizeof(SPECIFIC_KEYS) / sizeof(std::string); i++)
		if (key == SPECIFIC_KEYS[i])
			return true;
	return false;
}
class HttpParser;

class HttpRequest {
	public:
		HttpRequest() {};
		HttpRequest(HttpParser& parser);

		const std::string&	getMethod() const;
		const std::string&	getUri() const;
		const std::vector<std::pair<std::string, std::string> >& getHeaders() const;
		const std::vector<char>&	getBody() const;

	private:
		std::string	_method;
		std::string	_uri;
		std::vector<std::pair<std::string, std::string> >	_header;
		std::vector<char> _body;
		size_t	_contentLength;
	friend class HttpParser;
};

const std::string&	HttpRequest::getMethod() const {return _method;}
const std::string&	HttpRequest::getUri() const {return _uri;}
const std::vector<std::pair<std::string, std::string> >& HttpRequest::getHeaders() const {return _header;}
const std::vector<char>&	HttpRequest::getBody() const {return _body;}

class HttpParser {
	public:
		HttpParser();

		void			consume(const char* data, size_t len);
		std::string		getHeader(const std::string& key) const;
		HttpRequest*	generateRequest();

		bool	isComplete() const;
		void	reset();

		class	HttpRequestParsingException: public std::exception {
			public:
				HttpRequestParsingException(int code) : e_status(code) {};
			int	e_status;
		};

	private:
		ParserState			m_state;
		std::vector<char>	m_readBuf;
		size_t	m_cursor;
		size_t	m_contentLength;

		std::string			m_method;
		std::string			m_path;
		std::vector<std::pair<std::string, std::string> > m_header;
		std::vector<char>	m_body;

		bool	parseRequestLine();
		bool	parseHeader();
		bool	parseBody();
};

class Client {
	public:
	private:
};

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

//TODO add check for host
bool	HttpParser::parseHeader() {
	std::vector<char>::iterator	itStart = m_readBuf.begin() + m_cursor;
	std::vector<char>::iterator	itEndLine = std::search(itStart, m_readBuf.end(), "\r\n", ("\r\n") +2);

	if (itEndLine == m_readBuf.end())
		return false;

	if (itEndLine == itStart) {
		m_cursor += 2;
		std::string cl;
		if (!(cl = getHeader("transfer-encoding")).empty()) {
			if (cl != "chunked")
				throw HttpRequestParsingException(NOT_IMPLEMENTED);
			m_state = CHUNKEDBODY;
			return true;
		}
		else if (!(cl = getHeader("content-length")).empty()) {
			char*	endptr;
			long	val;
			val = std::strtol(cl.c_str(), &endptr, 10);
			if (*endptr != '\0' || val < 0 || val > MAX_BODY_SIZE)
				throw HttpRequestParsingException(BAD_REQUEST);
			m_contentLength = static_cast<size_t>(val);
			if (m_contentLength > 0) {
				m_state = BODY;
				m_body.reserve(m_contentLength);
				return true;
			}
		}
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


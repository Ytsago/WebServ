
#include <iostream>
#include <vector>

enum	ParserState {REQUEST_LINE, HEADER, BODY, COMPLETE};
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
	REQUEST_HEADER_FIELD_TOO_LARGE = 431
}

class HttpParser {
	public:
		void	consume(const char* data, size_t len);

		class	HttpRequestParsingException: public std::exception {
			HttpRequestParsingException() : e_status(0) {};
			HttpRequestParsingException(int code) : e_status(code) {};
			int	e_status;
		};
		class	HeaderTooBigException: public HttpRequestParsingException {
			HeaderTooBigException() : HttpRequestParsingException(REQUEST_HEADER_FIELD_TOO_LARGE) {};
		};

	private:
		ParserState			m_state;
		std::vector<char>	m_readBuf;
		size_t	m_offset;

		std::string			m_method;
		std::string			m_path;
		std::vector<std::pair<std::string, std::string> > m_header;
		std::vector<char>	m_body;
};

class HttpRequest {
	public:
	private:
};

class Client {
	public:
	private:
};

void	HttpParser::consume(const char* data, size_t len) {
	if (len + m_readBuf.size() > 8192 && m_state != BODY
		throw HeaderTooBigException();
	m_readBuf.insert(m_readBuf.begin(), data, data + len);
}

int main() {
	while(1) {
		char buffer[4096];
	}
}

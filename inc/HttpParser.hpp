#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "HttpRequest.hpp"
#include "HeaderMap.hpp"

# define MAX_BODY_SIZE 1000000 //Has to depend on the host server

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

class HttpParser {
	public:
		HttpParser();
		~HttpParser();

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

		enum	ParserState {REQUEST_LINE, HEADER, PROCESSHEADER, BODY, CHUNKEDBODY, COMPLETE};
		enum	MediaType {NONE, TEXT, APPLICATION, MULTIPART};

	private:
		HttpParser(const HttpParser& other);
		HttpParser&	operator=(const HttpParser& other);

		ParserState			m_state;
		MediaType			m_type;

		std::string			m_subtype;
		std::string			m_boundary;
		std::string			m_method;
		std::string			m_path;

		size_t				m_cursor;
		size_t				m_contentLength;
		size_t				m_bodySize;

		// std::vector<std::pair<std::string, std::string> > m_header;
		HeaderMap			m_headers;
		std::vector<char>	m_body;
		std::vector<char>	m_readBuf;

		bool	parseRequestLine();
		bool	parseHeader();
		bool	parseBody();

		void	processHeader();
		void	parseMediaType(std::string& media);

};

class multiPartParser {
	public:
		enum State{START, BOUND, HEADER, CONTENT, DONE};
	private:
		// State	m_state;
};

#endif

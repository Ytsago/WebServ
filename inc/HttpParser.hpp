#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "HttpRequest.hpp"
#include <vector>
#include <iostream>

# define MAX_BODY_SIZE 1000000 //Has to depend on the host server

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

		enum	ParserState {REQUEST_LINE, HEADER, BODY, CHUNKEDBODY, COMPLETE};

	private:
		HttpParser(const HttpParser& other);
		HttpParser&	operator=(const HttpParser& other);

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

#endif

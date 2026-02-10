#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <vector>
#include <iostream>

class HttpParser;

typedef std::vector<char> byteVector;

class HttpRequest {
	public:
		HttpRequest();
		HttpRequest(const HttpRequest& other);
		HttpRequest& operator=(const HttpRequest& other);
		~HttpRequest();

		const std::string&	getMethod() const;
		const std::string&	getUri() const;
		const std::vector<std::pair<std::string, std::string> >& getHeaders() const;
		const std::vector<char>&	getBody() const;
		std::string			getHeader(const std::string& key) const;

	private:
		std::string	_method;
		std::string	_uri;
		std::vector<std::pair<std::string, std::string> >	_header;
		std::vector<char> _body;
		size_t	_contentLength;
	friend class HttpParser;
	friend class RequestHandlerTester;
};

#endif

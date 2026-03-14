#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <vector>
#include "HeaderMap.hpp"
#include "ServerConfig.hpp"
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
		const HeaderMap& getHeaders() const;
		const std::vector<char>&	getBody() const;
		std::string			getHeader(const std::string& key) const;
		ServerConfig*		getHost();

		void	setHost(ServerConfig& host);

	private:
		std::string	_method;
		std::string	_uri;
		HeaderMap	_header;
		std::vector<char> _body;
		size_t	_contentLength;
		ServerConfig*	_host;
	friend class HttpParser;
	friend class RequestHandlerTester;
	friend class FileHandlerTester;
};

#endif

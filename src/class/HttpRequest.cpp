#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _contentLength(0) {
	_header.reserve(10);
}

HttpRequest::HttpRequest(const HttpRequest& other) :
	_method(other._method),
	_uri(other._uri),
	_header(other._header),
	_body(other._body),
	_contentLength(other._contentLength) {}

HttpRequest&	HttpRequest::operator=(const HttpRequest& other) {
	if (this != &other) {
		this->_method = other._method;
		this->_uri = other._uri;
		this->_header = other._header;
		this->_body = other._body;
		this->_contentLength = other._contentLength;
	}
	return *this;
}

std::string	HttpRequest::getHeader(const std::string& key) const 
{
	return this->_header[key];
}

const std::string&	HttpRequest::getMethod() const {return _method;}
const std::string&	HttpRequest::getUri() const {return _uri;}
const HeaderMap& HttpRequest::getHeaders() const {return _header;}
const std::vector<char>&	HttpRequest::getBody() const {return _body;}
ServerConfig*	HttpRequest::getHost() {return _host;}

void	HttpRequest::setHost(ServerConfig& host) {_host = &host;}
HttpRequest::~HttpRequest() {

};

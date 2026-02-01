#include <vector>
#include <iostream>

class HttpParser;

class HttpRequest {
	public:
		HttpRequest() {};
		HttpRequest(HttpParser& parser);
	private:
		std::string	_method;
		std::string	_uri;
		std::vector<std::pair<std::string, std::string> >	_header;
		std::vector<char> _body;
	friend class HttpParser;
};


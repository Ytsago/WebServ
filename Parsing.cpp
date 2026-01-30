
#include <iostream>
#include <vector>

enum ParserState {REQUEST_LINE, HEADER, BODY, COMPLETE};

class HttpParser {
	public:
		void	consume(const char* data, size_t len);

		class	HeaderTooBigException: public std::exception {
			virtual const char* what() const throw();
		};

	private:
		ParserState			m_state;
		std::vector<char>	m_readBuf;
		size_t	m_offset;

		std::string			m_method;
		std::string			m_path;
		std::vector<std::pair<std::string, std::string>> m_header;
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
	if (len + m_readBuf.size() > 8192 && m_state != BODY)
		throw HeaderTooBigException();
	m_readBuf.insert(m_readBuf.begin(), data, data + len);
}

int main() {
	int fd[2];

	pipe(fd);
	while(1) {
		char buffer[4096];
		read()
	}
}

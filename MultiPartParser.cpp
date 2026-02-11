#include "HttpRequest.hpp"
#include <fstream>
#include <unistd.h>
#include "Parser/inc/Parser.hpp"
#include "utils.hpp"

struct	PartData {
	int fd;
	std::string			filename;
	HeaderMap			headers;
	std::vector<char>	data;

	PartData(): fd(-1) {};
	~PartData() {if (fd > 0) close(fd);}
};

class MultiPartParser {
	public:
		MultiPartParser(const std::string& boundary);
		~MultiPartParser();

		void	consume(const char* data, size_t len);
		void	reset();
		
		enum State{START, BOUND, HEADER, CONTENT, DONE};
	private:
		State	m_state;
		size_t	m_cursor;
		std::vector<PartData>	m_part;
		const std::string	m_bound;

		
};

class PartDataAction: public IAction {
	virtual void	run(Token& T) const {
		std::string	filename = extractAttribute(T.toStr(), "filename=");
		if (!filename.empty())
			std::ofstream	outfile()
	}
};

void	MultiPartParser::consume(const char* data, size_t len) {
	Parser	bound(StringParser(m_bound.c_str()));
	Parser	boundSep(StringParser("--"));
	Parser	endLine(StringParser("\r\n"));

	Parser	parseLine = endLine.opt() + (boundSep + bound) + endLine;
	ParserResult<Token> res = parseLine.parse(data, len, 0);
	if (!res.success)
		throw std::runtime_error("BAD REQUEST");
	m_cursor = res.cursor;
}

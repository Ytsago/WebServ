#ifndef IPARSER_HPP
#define IPARSER_HPP

#include <string>

template <typename T>
struct	ParserResult {
	bool	success;
	T		value;
	size_t	cursor;
	ParserResult(bool s, T val, size_t i) : success(s), value(val), cursor(i) {}
	static ParserResult	fail(size_t i) {return ParserResult(false, T(), i);}
};

struct Token {
	const char *start;
	size_t	length;

	Token() : start(NULL), length(0) {};
	Token(const char* pt, size_t len) : start(pt), length(len) {}
	std::string	toStr() const {return std::string(start, length);}
};

class IParser {
public:
	virtual ~IParser() {}
	virtual	ParserResult<Token> parse(const char* data, size_t len, size_t pos) const = 0;
};

#endif

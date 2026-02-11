#ifndef BASICPARSERS_HPP
#define BASICPARSERS_HPP

#include "IParser.hpp"

class CharParser: public IParser {
	char	_target;
public:
	CharParser(char c) : _target(c) {};
	virtual ParserResult<Token>	parse(const char *data, size_t len, size_t pos) const;
};

class StringParser: public IParser {
	std::string _target;
	size_t		_length;
public:
	StringParser(const char* str): _target(str), _length(_target.size()) {};
	virtual ParserResult<Token> parse(const char* data, size_t len, size_t pos) const;
};

class InsStringParser: public IParser {
	std::string	_target;
	size_t		_length;
public:
	InsStringParser(const char* str): _target(str), _length(_target.size()) {
		for (size_t i = 0; i < _length; i++) _target[i] = std::tolower(_target[i]);}
	ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;
};

class DigitParser : public IParser {
public:
	DigitParser() {}
	virtual ParserResult<Token> parse(const char* data, size_t len, size_t pos) const;
};

class AlphaParser: public IParser {
public:
	AlphaParser() {}
	virtual ParserResult<Token> parse(const char* data, size_t len, size_t pos) const;
};

class AlnumParser: public IParser {
public:
	AlnumParser() {}
	virtual ParserResult<Token> parse(const char* data, size_t len, size_t pos) const;
};

class WhitespaceParser: public IParser {
public:
	WhitespaceParser() {}
	virtual ParserResult<Token> parse(const char* data, size_t len, size_t pos) const;
};

class RangeParser: public IParser {
	const char _min, _max;
public:
	RangeParser(char min, char max) : _min(min), _max(max) {}
	virtual ParserResult<Token> parse(const char* data, size_t len, size_t pos) const;
};

class AnyCharParser : public IParser {
public:
	AnyCharParser() {}

	virtual ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;
};

#endif

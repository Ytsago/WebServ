#ifndef COMBINATOR_HPP
#define COMBINATOR_HPP

#include "IParser.hpp"

class AndParser : public IParser {
	const IParser& _a;
	const IParser& _b;
public:
	AndParser(const IParser& a, const IParser& b) : _a(a), _b(b) {}
	virtual ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;
};

class OrParser : public IParser {
	const IParser& _a;
	const IParser& _b;
public:
	OrParser(const IParser& a, const IParser& b) : _a(a), _b(b) {}
	virtual ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;
};

class ManyParser:public IParser {
	const IParser&	_child;
	bool	_moreThanOne;
public:
	ManyParser(const IParser& a, bool moreThanOne = false) : _child(a), _moreThanOne(moreThanOne) {}
	virtual ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;
};

class UntilParser: public IParser {
	const IParser&	_child;
public:
	UntilParser(const IParser& a) : _child(a) {}
	virtual ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;
};

class OptionalParser: public IParser {
	const IParser&	_child;
public:
	OptionalParser(const IParser& a) : _child(a) {}
	virtual ParserResult<Token> parse(const char* data, size_t len, size_t pos) const;
};

class NotParser: public IParser {
	const IParser&	_forbidden;
public:
	NotParser(const IParser& a) : _forbidden(a) {}

	virtual ParserResult<Token>	parse(const char *data, size_t len, size_t pos) const;
};

#endif

#ifndef PARSER_HPP
#define PARSER_HPP

#include "ActionParser.hpp"
#include "ParserManager.hpp"
#include "Combinator.hpp"
#include "BasicParsers.hpp"

class Parser: public IParser {
	const IParser*	_ptr;
	static ParserManager	_manager;
public:
	Parser(const IParser* p = NULL);
	Parser(const IParser& p);
	
	virtual ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;

	Parser	operator+(const Parser& other) const;
	Parser	operator|(const Parser& other) const;

	Parser	operator*() const;
	Parser	operator+() const;

	Parser	repeat(bool	atLeastOne = false) const;
	Parser	action(IAction& act) const;
	Parser	opt() const;
};

#endif

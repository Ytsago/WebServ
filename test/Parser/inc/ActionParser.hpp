#ifndef ACTIONPARSER_HPP
#define ACTIONPARSER_HPP

#include "IAction.hpp"

class ActionParser : public IParser {
	const IParser&	_child;
	const IAction&	_action;
public:
	ActionParser(const IParser& a, const IAction& act) : _child(a), _action(act) {}

	virtual ParserResult<Token>	parse(const char* data, size_t len, size_t pos) const;
};

#endif

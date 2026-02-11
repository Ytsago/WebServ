#include "ActionParser.hpp"

ParserResult<Token>	ActionParser::parse(const char* data, size_t len, size_t pos) const {
	ParserResult<Token> res = _child.parse(data, len, pos);
	if (res.success)
		_action.run(res.value);
	return res;
}

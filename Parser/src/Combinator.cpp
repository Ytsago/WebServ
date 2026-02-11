#include "Combinator.hpp"

ParserResult<Token>	AndParser::parse(const char* data, size_t len, size_t pos) const {
	ParserResult<Token> resA = _a.parse(data, len, pos);
	if (!resA.success) return resA;

	ParserResult<Token> resB = _b.parse(data, len, resA.cursor);
	if (!resB.success) return resB;

	return ParserResult<Token>(true,
							Token(resA.value.start, resA.value.length + resB.value.length),
							resB.cursor);
}

ParserResult<Token>	OrParser::parse(const char* data, size_t len, size_t pos) const {
	ParserResult<Token> res = _a.parse(data, len, pos);
	if (res.success) return res;
	res = _b.parse(data, len, pos);
	if (res.success) return res;

	return ParserResult<Token>::fail(pos);
}

ParserResult<Token>	ManyParser::parse(const char* data, size_t len, size_t pos) const {
	size_t	count = 0;
	size_t	currPos = pos;

	while (true) {
		ParserResult<Token> res = _child.parse(data, len, currPos);
		if (!res.success) break ;

		if (currPos == res.cursor) break ;
		currPos = res.cursor;
		count++;
	}

	if (count == 0 && _moreThanOne)
		return ParserResult<Token>::fail(pos);
	return ParserResult<Token>(true, Token(data + pos, currPos - pos), currPos);
}

ParserResult<Token>	UntilParser::parse(const char* data, size_t len, size_t pos) const {
	size_t	currPos = pos;
	while (currPos < len) {
		ParserResult<Token>	res = _child.parse(data, len, currPos);
		if (res.success) return ParserResult<Token>(true, Token(data + pos, currPos - pos), currPos);
		currPos++;
	}
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token> OptionalParser::parse(const char* data, size_t len, size_t pos) const {
	ParserResult<Token> res = _child.parse(data, len, pos);
	if (res.success) return res;

	return ParserResult<Token>(true, Token(data + pos, 0), pos);
}

ParserResult<Token>	NotParser::parse(const char *data, size_t len, size_t pos) const {
	ParserResult<Token> res = _forbidden.parse(data, len, pos);
	if (res.success) return ParserResult<Token>::fail(pos);
	return ParserResult<Token>(true, Token(data + pos, 0), pos);
}

#include "BasicParsers.hpp"
#include <cstring>

ParserResult<Token>	CharParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos < len && data[pos] == _target)
		return ParserResult<Token>(true, Token(&data[pos], 1), pos + 1);
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token>	StringParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos + _length <= len && strncmp(_target.c_str(), &data[pos], _length) == 0)
		return ParserResult<Token>(true, Token(&data[pos], _length), pos + _length);
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token>	InsStringParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos + _length <= len) {
		std::string	value(data + pos, _length);
		for (size_t i = 0; i < _length; i++) value[i] = std::tolower(value[i]);
		if (value == _target)
			return ParserResult<Token>(true, Token(data + pos, _length), pos + _length);
	}
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token> DigitParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos < len && std::isdigit(data[pos]))
		return ParserResult<Token>(true, Token(&data[pos], 1), pos + 1);
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token> AlphaParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos < len && std::isalpha(data[pos]))
		return ParserResult<Token>(true, Token(data + pos, 1), pos + 1);
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token> AlnumParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos < len && std::isalnum(data[pos]))
		return ParserResult<Token>(true, Token(data + pos, 1), pos + 1);
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token> WhitespaceParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos < len && std::isspace(data[pos]))
		return ParserResult<Token>(true, Token(data + pos, 1), pos + 1);
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token> RangeParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos < len && (data[pos] >= _min && data[pos] <= _max))
		return ParserResult<Token>(true, Token(data + pos, 1), pos + 1);
	return ParserResult<Token>::fail(pos);
}

ParserResult<Token>	AnyCharParser::parse(const char* data, size_t len, size_t pos) const {
	if (pos < len)
		return ParserResult<Token>(true, Token(data + pos, 1), pos +1);
	return ParserResult<Token>::fail(pos);
}

#include "Parser.hpp"
#include "Combinator.hpp"

ParserManager	Parser::_manager;

Parser::Parser(const IParser* p) : _ptr(p) {}
Parser::Parser(const IParser& p) : _ptr(&p) {}


ParserResult<Token>	Parser::parse(const char* data, size_t len, size_t pos) const {
	if (!_ptr) return ParserResult<Token>::fail(pos);
	return _ptr->parse(data, len, pos);
}

Parser	Parser::operator+(const Parser& other) const {
	return Parser(_manager.track(new AndParser(*this->_ptr, *other._ptr)));
}

Parser	Parser::operator|(const Parser& other) const {
	return Parser(_manager.track(new OrParser(*this->_ptr, *other._ptr)));
}

Parser	Parser::operator*() const {
	return this->repeat(false);
}

Parser	Parser::operator+() const {
	return this->repeat(true);
}

Parser	Parser::repeat(bool	atLeastOne) const {
	return Parser(_manager.track(new ManyParser(*this->_ptr, atLeastOne)));
}

Parser	Parser::action(IAction& act) const {
	return Parser(_manager.track(new ActionParser(*this->_ptr, act)));
}

Parser Parser::opt() const {
	return Parser(_manager.track(new OptionalParser(*this->_ptr)));
}

//
// int main() {
// 	const char	str[] = "Hello world";
//
// 	Parser	match = StringParser("world");
//
// 	Parser	line = Parser(UntilParser(match));
// 	std::string result =  line.parse(str, sizeof(str), 0).value.toStr();
// 	std::cout << result;
// }

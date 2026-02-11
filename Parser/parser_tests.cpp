#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "Parser.hpp"

class CollectAction : public IAction {
public:
	mutable std::vector<std::string> collected;
	
	void run(Token& t) const {
		collected.push_back(t.toStr());
	}
};
// ============================================================================
// TEST SUITE
// ============================================================================

void test_char_parser() {
	std::cout << "\n=== Testing CharParser ===" << std::endl;
	
	CharParser parser('a');
	const char* input = "abc";
	
	// Test success
	ParserResult<Token> res = parser.parse(input, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "a");
	assert(res.cursor == 1);
	std::cout << "✓ CharParser matches correct character" << std::endl;
	
	// Test failure
	res = parser.parse(input, 3, 1); // 'b' position
	assert(res.success == false);
	std::cout << "✓ CharParser fails on wrong character" << std::endl;
	
	// Test bounds
	res = parser.parse(input, 3, 3); // Out of bounds
	assert(res.success == false);
	std::cout << "✓ CharParser handles out of bounds" << std::endl;
}

void test_string_parser() {
	std::cout << "\n=== Testing StringParser ===" << std::endl;
	
	StringParser parser("hello");
	const char* input = "hello world";
	
	// Test success
	ParserResult<Token> res = parser.parse(input, 11, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "hello");
	assert(res.cursor == 5);
	std::cout << "✓ StringParser matches correct string" << std::endl;
	
	// Test failure
	const char* input2 = "hell world";
	res = parser.parse(input2, 10, 0);
	assert(res.success == false);
	std::cout << "✓ StringParser fails on partial match" << std::endl;
	
	// Test at different position
	const char* input3 = "say hello there";
	res = parser.parse(input3, 15, 4);
	assert(res.success == true);
	assert(res.value.toStr() == "hello");
	std::cout << "✓ StringParser works at non-zero position" << std::endl;
}

void test_digit_parser() {
	std::cout << "\n=== Testing DigitParser ===" << std::endl;
	
	DigitParser parser;
	const char* input = "5abc";
	
	// Test success
	ParserResult<Token> res = parser.parse(input, 4, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "5");
	assert(res.cursor == 1);
	std::cout << "✓ DigitParser matches digit" << std::endl;
	
	// Test failure
	res = parser.parse(input, 4, 1); // 'a' position
	assert(res.success == false);
	std::cout << "✓ DigitParser fails on non-digit" << std::endl;
	
	// Test all digits
	const char* digits = "0123456789";
	for (int i = 0; i < 10; i++) {
		res = parser.parse(digits, 10, i);
		assert(res.success == true);
	}
	std::cout << "✓ DigitParser matches all digits 0-9" << std::endl;
}

void test_and_parser() {
	std::cout << "\n=== Testing AndParser ===" << std::endl;
	
	CharParser a('a');
	CharParser b('b');
	AndParser parser(a, b);
	
	// Test success
	const char* input = "abc";
	ParserResult<Token> res = parser.parse(input, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "ab");
	assert(res.cursor == 2);
	std::cout << "✓ AndParser matches sequence" << std::endl;
	
	// Test first fails
	const char* input2 = "bbc";
	res = parser.parse(input2, 3, 0);
	assert(res.success == false);
	std::cout << "✓ AndParser fails when first parser fails" << std::endl;
	
	// Test second fails
	const char* input3 = "acc";
	res = parser.parse(input3, 3, 0);
	assert(res.success == false);
	std::cout << "✓ AndParser fails when second parser fails" << std::endl;
}

void test_or_parser() {
	std::cout << "\n=== Testing OrParser ===" << std::endl;
	
	CharParser a('a');
	CharParser b('b');
	OrParser parser(a, b);
	
	// Test first matches
	const char* input = "abc";
	ParserResult<Token> res = parser.parse(input, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "a");
	std::cout << "✓ OrParser matches first alternative" << std::endl;
	
	// Test second matches
	const char* input2 = "bbc";
	res = parser.parse(input2, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "b");
	std::cout << "✓ OrParser matches second alternative" << std::endl;
	
	// Test both fail
	const char* input3 = "ccc";
	res = parser.parse(input3, 3, 0);
	assert(res.success == false);
	std::cout << "✓ OrParser fails when both alternatives fail" << std::endl;
}

void test_many_parser() {
	std::cout << "\n=== Testing ManyParser ===" << std::endl;
	
	DigitParser digit;
	ManyParser parser(digit, false);
	
	// Test multiple matches
	const char* input = "12345abc";
	ParserResult<Token> res = parser.parse(input, 8, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "12345");
	assert(res.cursor == 5);
	std::cout << "✓ ManyParser matches multiple items" << std::endl;
	
	// Test zero matches (should succeed with moreThanOne=false)
	const char* input2 = "abc";
	res = parser.parse(input2, 3, 0);
	assert(res.success == true);
	assert(res.value.length == 0);
	std::cout << "✓ ManyParser succeeds with zero matches (moreThanOne=false)" << std::endl;
	
	// Test with moreThanOne=true
	ManyParser parser2(digit, true);
	res = parser2.parse(input2, 3, 0);
	assert(res.success == false);
	std::cout << "✓ ManyParser fails with zero matches (moreThanOne=true)" << std::endl;
	
	res = parser2.parse(input, 8, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "12345");
	std::cout << "✓ ManyParser succeeds with matches (moreThanOne=true)" << std::endl;
}

void test_until_parser() {
	std::cout << "\n=== Testing UntilParser ===" << std::endl;
	
	StringParser terminator("END");
	UntilParser parser(terminator);
	
	// Test normal case
	const char* input = "Hello WorldEND";
	ParserResult<Token> res = parser.parse(input, 14, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "Hello World");
	assert(res.cursor == 11);
	std::cout << "✓ UntilParser parses until terminator" << std::endl;
	
	// Test terminator not found
	const char* input2 = "Hello World";
	res = parser.parse(input2, 11, 0);
	assert(res.success == false);
	std::cout << "✓ UntilParser fails when terminator not found" << std::endl;
	
	// Test immediate match
	const char* input3 = "ENDHello";
	res = parser.parse(input3, 8, 0);
	assert(res.success == true);
	assert(res.value.length == 0);
	std::cout << "✓ UntilParser handles immediate terminator" << std::endl;
}

void test_parser_operators() {
	std::cout << "\n=== Testing Parser Operators ===" << std::endl;
	
	CharParser a('a');
	CharParser b('b');
	CharParser c('c');
	
	// Test + operator (AndParser)
	Parser pAnd = Parser(a) + Parser(b);
	const char* input = "abc";
	ParserResult<Token> res = pAnd.parse(input, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "ab");
	std::cout << "✓ Parser operator+ works" << std::endl;
	
	// Test | operator (OrParser)
	Parser pOr = Parser(a) | Parser(b);
	res = pOr.parse(input, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "a");
	
	const char* input2 = "bac";
	res = pOr.parse(input2, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "b");
	std::cout << "✓ Parser operator| works" << std::endl;
	
	// Test * operator (ManyParser with moreThanOne=false)
	DigitParser digit;
	Parser pMany = *Parser(digit);
	const char* input3 = "123abc";
	res = pMany.parse(input3, 6, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "123");
	std::cout << "✓ Parser operator* works" << std::endl;
	
	// Test + operator (ManyParser with moreThanOne=true)
	Parser pMany1 = +Parser(digit);
	res = pMany1.parse(input3, 6, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "123");
	
	const char* input4 = "abc";
	res = pMany1.parse(input4, 3, 0);
	assert(res.success == false);
	std::cout << "✓ Parser unary operator+ works" << std::endl;
}

void test_action_parser() {
	std::cout << "\n=== Testing ActionParser ===" << std::endl;
	
	CollectAction action;
	CharParser a('a');
	ActionParser parser(a, action);
	
	const char* input = "abc";
	ParserResult<Token> res = parser.parse(input, 3, 0);
	
	assert(res.success == true);
	assert(action.collected.size() == 1);
	assert(action.collected[0] == "a");
	std::cout << "✓ ActionParser executes action on success" << std::endl;
	
	// Test action not called on failure
	CollectAction action2;
	ActionParser parser2(a, action2);
	res = parser2.parse(input, 3, 1); // 'b' position
	
	assert(res.success == false);
	assert(action2.collected.size() == 0);
	std::cout << "✓ ActionParser doesn't execute action on failure" << std::endl;
}

void test_complex_combination() {
	std::cout << "\n=== Testing Complex Parser Combinations ===" << std::endl;
	
	// Parse a simple number: optional sign + digits
	CharParser plus('+');
	CharParser minus('-');
	DigitParser digit;
	
	Parser sign = Parser(plus) | Parser(minus);
	Parser digits = +Parser(digit);
	Parser number = (*Parser(sign)) + digits;
	
	// Test positive number
	const char* input1 = "+123";
	ParserResult<Token> res = number.parse(input1, 4, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "+123");
	std::cout << "✓ Complex parser handles +123" << std::endl;
	
	// Test negative number
	const char* input2 = "-456";
	res = number.parse(input2, 4, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "-456");
	std::cout << "✓ Complex parser handles -456" << std::endl;
	
	// Test unsigned number
	const char* input3 = "789";
	res = number.parse(input3, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "789");
	std::cout << "✓ Complex parser handles 789" << std::endl;
}

void test_chained_operations() {
	std::cout << "\n=== Testing Chained Operations ===" << std::endl;
	
	CharParser a('a');
	CharParser b('b');
	CharParser c('c');
	
	// Chain multiple operations
	Parser complex = (Parser(a) + Parser(b)) | Parser(c);
	
	const char* input1 = "abc";
	ParserResult<Token> res = complex.parse(input1, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "ab");
	std::cout << "✓ Chained (a+b)|c matches 'ab'" << std::endl;
	
	const char* input2 = "cba";
	res = complex.parse(input2, 3, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "c");
	std::cout << "✓ Chained (a+b)|c matches 'c'" << std::endl;
}

void test_edge_cases() {
	std::cout << "\n=== Testing Edge Cases ===" << std::endl;
	
	// Empty input
	CharParser a('a');
	ParserResult<Token> res = a.parse("", 0, 0);
	assert(res.success == false);
	std::cout << "✓ Handles empty input" << std::endl;
	
	// Position at end
	const char* input = "abc";
	res = a.parse(input, 3, 3);
	assert(res.success == false);
	std::cout << "✓ Handles position at end" << std::endl;
	
	// Zero-length string parser
	StringParser empty("");
	res = empty.parse(input, 3, 0);
	assert(res.success == true);
	assert(res.value.length == 0);
	std::cout << "✓ Handles empty string parser" << std::endl;
}

void test_real_world_example() {
	std::cout << "\n=== Testing Real-World Example ===" << std::endl;
	
	// Parse a simple CSV line: number,word,number
	DigitParser digit;
	CharParser comma(',');
	CharParser letter_a('a');
	CharParser letter_z('z');
	
	Parser digits = +Parser(digit);
	Parser word = +(Parser(letter_a) | Parser(letter_z));
	Parser csv_line = digits + Parser(comma) + word + Parser(comma) + digits;
	
	const char* input = "123,azaz,456";
	ParserResult<Token> res = csv_line.parse(input, 12, 0);
	assert(res.success == true);
	assert(res.value.toStr() == "123,azaz,456");
	std::cout << "✓ Real-world CSV parser works" << std::endl;
}

int main() {
	std::cout << "========================================" << std::endl;
	std::cout << "  Parser Combinator Library Test Suite" << std::endl;
	std::cout << "========================================" << std::endl;
	
	try {
		test_char_parser();
		test_string_parser();
		test_digit_parser();
		test_and_parser();
		test_or_parser();
		test_many_parser();
		test_until_parser();
		test_parser_operators();
		test_action_parser();
		test_complex_combination();
		test_chained_operations();
		test_edge_cases();
		test_real_world_example();
		
		std::cout << "\n========================================" << std::endl;
		std::cout << "  ✓ ALL TESTS PASSED!" << std::endl;
		std::cout << "========================================" << std::endl;
		
	} catch (const std::exception& e) {
		std::cerr << "\n✗ TEST FAILED: " << e.what() << std::endl;
		return 1;
	}
	
	return 0;
}

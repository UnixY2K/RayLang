#pragma once

#include <cstddef>
#include <ray/compiler/token.hpp>

#include <string_view>
#include <vector>

namespace ray::compiler {
class Tokenizer {
	std::string_view source;
	std::vector<Token> tokens;
	size_t start;
	size_t current;
	size_t line;

  public:
	Tokenizer(std::string_view source);

	bool isAtEnd();
	std::vector<Token> scanTokens();

  private:
	void scanToken();
	char advance();

	void addToken(Token::TokenType type);
	void addToken(Token::TokenType type, std::string literal);
	bool match(char expected);
	void string();
	bool isDigit(char c);
	void number();
	bool isAlpha(char c);
	bool isAlphaNumeric(char c);
	void identifier();
	char peek();
	char peekNext();

};

} // namespace ray::compiler

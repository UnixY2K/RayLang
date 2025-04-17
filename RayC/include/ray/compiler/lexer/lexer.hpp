#pragma once

#include <ray/compiler/lexer/lexer_error.hpp>
#include <ray/compiler/lexer/token.hpp>

#include <cstddef>
#include <string_view>
#include <vector>

namespace ray::compiler {
class Lexer {
	std::string_view source;
	std::vector<Token> tokens;
	std::vector<LexerError> errors;
	size_t start = 0;
	size_t current = 0;
	size_t line = 0;
	size_t column = 0;

  public:
	Lexer(std::string_view source);

	bool isAtEnd() const;
	std::vector<Token> scanTokens();

	const std::vector<LexerError>& getErrors() const;

  private:
	void scanToken();
	char advance();

	void addToken(Token::TokenType type);
	void addToken(Token::TokenType type, std::string literal);
	bool match(char expected);
	void string();
	void number();
	void identifier();
	void comment();

	char peek();
	char peekNext();
};

} // namespace ray::compiler

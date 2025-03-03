#include <ray/compiler/lexer/token.hpp>

#include <format>

namespace ray::compiler {
std::string Token::toString() const {
	return std::format("Token{{type: {}, lexeme: '{}', line: {}}}",
	                   toString(type), lexeme, line);
}

Token::TokenType Token::fromString(std::string_view str) {}

std::string_view Token::toString(TokenType token) {
	switch (token) {
	// NON Used, only to hint that the token is not initialized
	case TokenType::TOKEN_UNINITIALIZED:
		return "TOKEN_UNINITIALIZED";
	// block tokens
	case TokenType::TOKEN_LEFT_PAREN:
		return "TOKEN_LEFT_PAREN";
	case TokenType::TOKEN_RIGHT_PAREN:
		return "TOKEN_RIGHT_PAREN";
	case TokenType::TOKEN_LEFT_BRACE:
		return "TOKEN_LEFT_BRACE";
	case TokenType::TOKEN_RIGHT_BRACE:
		return "TOKEN_RIGHT_BRACE";
	case TokenType::TOKEN_LEFT_SQUARE_BRACE:
		return "TOKEN_LEFT_SQUARE_BRACE";
	case TokenType::TOKEN_RIGHT_SQUARE_BRACE:
		return "TOKEN_RIGHT_SQUARE_BRACE";
	// assignment
	case TokenType::TOKEN_EQUAL:
		return "TOKEN_EQUAL";
	case TokenType::TOKEN_PLUS_EQUAL:
		return "TOKEN_PLUS_EQUAL";
	case TokenType::TOKEN_MINUS_EQUAL:
		return "TOKEN_MINUS_EQUAL";
	case TokenType::TOKEN_STAR_EQUAL:
		return "TOKEN_STAR_EQUAL";
	case TokenType::TOKEN_SLASH_EQUAL:
		return "TOKEN_SLASH_EQUAL";
	case TokenType::TOKEN_PERCENT_EQUAL:
		return "TOKEN_PERCENT_EQUAL";
	case TokenType::TOKEN_AMPERSAND_EQUAL:
		return "TOKEN_AMPERSAND_EQUAL";
	case TokenType::TOKEN_PIPE_EQUAL:
		return "TOKEN_PIPE_EQUAL";
	case TokenType::TOKEN_CARET_EQUAL:
		return "TOKEN_CARET_EQUAL";
	case TokenType::TOKEN_LESS_LESS_EQUAL:
		return "TOKEN_LESS_LESS_EQUAL";
	case TokenType::TOKEN_GREAT_GREAT_EQUAL:
		return "TOKEN_GREAT_GREAT_EQUAL";
	// increment, decrement
	case TokenType::TOKEN_PLUS_PLUS:
		return "TOKEN_PLUS_PLUS";
	case TokenType::TOKEN_MINUS_MINUS:
		return "TOKEN_MINUS_MINUS";
	// arithmetic
	case TokenType::TOKEN_PLUS:
		return "TOKEN_PLUS";
	case TokenType::TOKEN_MINUS:
		return "TOKEN_MINUS";
	case TokenType::TOKEN_STAR:
		return "TOKEN_STAR";
	case TokenType::TOKEN_SLASH:
		return "TOKEN_SLASH";
	case TokenType::TOKEN_PERCENT:
		return "TOKEN_PERCENT";
	case TokenType::TOKEN_AMPERSAND:
		return "TOKEN_AMPERSAND";
	case TokenType::TOKEN_PIPE:
		return "TOKEN_PIPE";
	case TokenType::TOKEN_CARET:
		return "TOKEN_CARET";
	case TokenType::TOKEN_LEFT_SHIFT:
		return "TOKEN_LEFT_SHIFT";
	case TokenType::TOKEN_RIGHT_SHIFT:
		return "TOKEN_RIGHT_SHIFT";
	// logical
	case TokenType::TOKEN_BANG:
		return "TOKEN_BANG";
	case TokenType::TOKEN_AMPERSAND_AMPERSAND:
		return "TOKEN_AMPERSAND_AMPERSAND";
	case TokenType::TOKEN_PIPE_PIPE:
		return "TOKEN_PIPE_PIPE";
	// comparison
	case TokenType::TOKEN_EQUAL_EQUAL:
		return "TOKEN_EQUAL_EQUAL";
	case TokenType::TOKEN_BANG_EQUAL:
		return "TOKEN_BANG_EQUAL";
	case TokenType::TOKEN_LESS:
		return "TOKEN_LESS";
	case TokenType::TOKEN_GREAT:
		return "TOKEN_GREAT";
	case TokenType::TOKEN_LESS_EQUAL:
		return "TOKEN_LESS_EQUAL";
	case TokenType::TOKEN_GREAT_EQUAL:
		return "TOKEN_GREAT_EQUAL";
	// MISC
	case TokenType::TOKEN_DOT:
		return "TOKEN_DOT";
	case TokenType::TOKEN_COMMA:
		return "TOKEN_COMMA";
	case TokenType::TOKEN_QUESTION:
		return "TOKEN_QUESTION";
	// Literals.
	case TokenType::TOKEN_IDENTIFIER:
		return "TOKEN_IDENTIFIER";
	case TokenType::TOKEN_STRING:
		return "TOKEN_STRING";
	case TokenType::TOKEN_NUMBER:
		return "TOKEN_NUMBER";
	// keywords
	case TokenType::TOKEN_IF:
		return "TOKEN_IF";
	case TokenType::TOKEN_ELSE:
		return "TOKEN_ELSE";
	case TokenType::TOKEN_TRUE:
		return "TOKEN_TRUE";
	case TokenType::TOKEN_FALSE:
		return "TOKEN_FALSE";
	case TokenType::TOKEN_FOR:
		return "TOKEN_FOR";
	case TokenType::TOKEN_WHILE:
		return "TOKEN_WHILE";
	case TokenType::TOKEN_FN:
		return "TOKEN_FN";
	case TokenType::TOKEN_RETURN:
		return "TOKEN_RETURN";
	case TokenType::TOKEN_CONTINUE:
		return "TOKEN_CONTINUE";
	case TokenType::TOKEN_BREAK:
		return "TOKEN_BREAK";
	// other
	case TokenType::TOKEN_ERROR:
		return "TOKEN_ERROR";
	case TokenType::TOKEN_EOF:
		return "TOKEN_EOF";
	}
	return "???";
}

} // namespace ray::compiler

#include <cctype>
#include <cstddef>
#include <format>
#include <string_view>
#include <vector>

#include <ray/compiler/lexer/lexer.hpp>
#include <ray/compiler/lexer/lexer_error.hpp>
#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler {

Lexer::Lexer(std::string_view source) : source(source) {}

bool Lexer::isAtEnd() const { return current >= source.length(); };

std::vector<Token> Lexer::scanTokens() {
	tokens.clear();
	errors.clear();
	start = 0;
	current = 0;
	line = 1;
	column = 0;

	while (!isAtEnd()) {
		start = current;
		scanToken();
	}

	addToken(Token::TokenType::TOKEN_EOF);

	return tokens;
}

const std::vector<LexerError> &Lexer::getErrors() const { return errors; }

void Lexer::scanToken() {
	char c = advance();
	Token::TokenType type = Token::fromChar(c);
	switch (type) {
	// multi char tokens
	case Token::TokenType::TOKEN_PLUS: {
		char next = peek();
		if (next == '+') {
			advance();
			addToken(Token::TokenType::TOKEN_PLUS_PLUS);
		} else if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_PLUS_EQUAL);
		} else {
			addToken(Token::TokenType::TOKEN_PLUS);
		}
		break;
	}
	case Token::TokenType::TOKEN_MINUS: {
		char next = peek();
		if (next == '>') {
			advance();
			addToken(Token::TokenType::TOKEN_ARROW);
		} else if (next == '-') {
			advance();
			addToken(Token::TokenType::TOKEN_MINUS_MINUS);
		} else if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_MINUS_EQUAL);
		} else {
			addToken(Token::TokenType::TOKEN_MINUS);
		}
		break;
	}
	case Token::TokenType::TOKEN_STAR: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_STAR_EQUAL);
		} else {
			addToken(Token::TokenType::TOKEN_STAR);
		}
		break;
	}
	case Token::TokenType::TOKEN_SLASH: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_SLASH_EQUAL);
		} else if (next == '/') {
			advance();
			comment();
		} else if (next == '*') {
			advance();
			multiLineComment();
		} else {
			addToken(Token::TokenType::TOKEN_SLASH);
		}
		break;
	}
	case Token::TokenType::TOKEN_PERCENT: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_PERCENT_EQUAL);
		} else {
			addToken(Token::TokenType::TOKEN_PERCENT);
		}
		break;
	}
	case Token::TokenType::TOKEN_AMPERSAND: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_AMPERSAND_EQUAL);
		} else if (next == '&') {
			advance();
			addToken(Token::TokenType::TOKEN_AMPERSAND_AMPERSAND);
		} else {
			addToken(Token::TokenType::TOKEN_AMPERSAND);
		}
		break;
	}
	case Token::TokenType::TOKEN_PIPE: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_PIPE_EQUAL);
		} else if (next == '|') {
			advance();
			addToken(Token::TokenType::TOKEN_PIPE_PIPE);
		} else {
			addToken(Token::TokenType::TOKEN_PIPE);
		}
		break;
	}
	case Token::TokenType::TOKEN_CARET: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_CARET_EQUAL);
		} else {
			addToken(Token::TokenType::TOKEN_CARET);
		}
		break;
	}
	case Token::TokenType::TOKEN_BANG: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_BANG_EQUAL);
		} else {
			addToken(Token::TokenType::TOKEN_BANG);
		}
		break;
	}
	case Token::TokenType::TOKEN_LESS: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_LESS_EQUAL);
		} else if (next == '<') {
			advance();
			if (peek() == '=') {
				advance();
				addToken(Token::TokenType::TOKEN_LESS_LESS_EQUAL);
			} else {
				addToken(Token::TokenType::TOKEN_LESS_LESS);
			}
		} else {
			addToken(Token::TokenType::TOKEN_LESS);
		}
		break;
	}
	case Token::TokenType::TOKEN_GREAT: {
		char next = peek();
		if (next == '=') {
			advance();
			addToken(Token::TokenType::TOKEN_GREAT_EQUAL);
		} else if (next == '>') {
			advance();
			if (peek() == '=') {
				advance();
				addToken(Token::TokenType::TOKEN_GREAT_GREAT_EQUAL);
			} else {
				addToken(Token::TokenType::TOKEN_GREAT_GREAT);
			}
		} else {
			addToken(Token::TokenType::TOKEN_LESS);
		}
		break;
	}
	// access or number
	case Token::TokenType::TOKEN_DOT: {
		char next = peekNext();
		if (std::isdigit(next)) {
			number();
		} else {
			addToken(Token::TokenType::TOKEN_DOT);
		}
		break;
	}
	// not expected character
	case Token::TokenType::TOKEN_ERROR: {
		if (std::isalpha(c) || c == '_') {
			identifier();
		} else if (std::isdigit(c)) {
			number();
		} else if (c == '"') {
			string();
		} else if (c == '\'') {
			charLiteral();
		} else if (c == '@') {
			intrinsicFunction();
		} else if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
		} else {
			Token errorToken =
			    Token{type, std::string{std::string_view(&c, 1)}, line, column};
			errors.push_back(
			    {.category = LexerError::ErrorCategory::UnexpectedCharacter,
			     .token = errorToken,
			     .message = "Unexpected character"});
		}
		break;
	}
	// any other token
	default: {
		addToken(type);
		break;
	}
	}
}

char Lexer::advance() {
	column++;
	if (peek() == '\n') {
		line++;
		column = 0;
	}
	return source[current++];
};

void Lexer::addToken(Token::TokenType type) { addToken(type, ""); }
void Lexer::addToken(Token::TokenType type, std::string literal) {
	tokens.push_back(
	    Token{.type = type, .lexeme = literal, .line = line, .column = column});
}
void Lexer::addToken(Token::TokenType type, std::string literal, size_t line,
                     size_t column) {
	tokens.push_back(
	    Token{.type = type, .lexeme = literal, .line = line, .column = column});
}

bool Lexer::match(char expected) {
	if (isAtEnd() || source[current] != expected) {
		return false;
	}
	current++;
	return true;
}

void Lexer::string() {
	std::string value;
	while (peek() != '"' && !isAtEnd()) {
		// escape characters starting with backslash
		if (peek() == '\\') {
			advance();
			if (isAtEnd()) {
				Token errorToken{Token::TokenType::TOKEN_STRING, value, line,
				                 column};
				errors.push_back(
				    {.category = LexerError::ErrorCategory::UnterminatedString,
				     .token = errorToken,
				     .message = "Unterminated string"});
			}
			switch (peek()) {
			case 'n': {
				value.push_back('\n');
				break;
			}
			case '0': {
				value.push_back('\0');
				break;
			}
			default: {
				Token errorToken{Token::TokenType::TOKEN_STRING, value, line,
				                 column};
				errors.push_back(
				    {.category = LexerError::ErrorCategory::UnterminatedString,
				     .token = errorToken,
				     .message = std::format("Unknown escape sequence '\\{}'",
				                            peek())});
				return;
			}
			}
			advance();
			continue;
		}
		value.push_back(peek());
		advance();
	}

	if (isAtEnd()) {
		Token errorToken{Token::TokenType::TOKEN_STRING, value, line, column};
		errors.push_back(
		    {.category = LexerError::ErrorCategory::UnterminatedString,
		     .token = errorToken,
		     .message = "Unterminated string"});
		return;
	}

	advance();

	addToken(Token::TokenType::TOKEN_STRING, value);
}

void Lexer::number() {
	while (std::isdigit(peek())) {
		advance();
	}

	if (peek() == '.' && std::isdigit(peekNext())) {
		advance();
		while (std::isdigit(peek())) {
			advance();
		}
	}

	auto number_literal = source.substr(start, current - start);

	if (peek() == 'i' || peek() == 'u') {
		size_t index = number_literal.length();
		std::string_view new_literal = "";
		auto subType = source.substr(start + index, 3);

		if ((subType[1] == '1' && subType[2] == '6') ||
		    (subType[1] == '3' && subType[2] == '2') ||
		    (subType[1] == '6' && subType[2] == '4')) {
			new_literal = source.substr(start, current - start + 3);
		} else if (subType[1] == '8') {
			new_literal = source.substr(start, current - start + 2);
		}

		auto nextChar = source[start + new_literal.length()];
		if (!std::isalnum(nextChar) && nextChar != '_') {
			number_literal = new_literal;
		}

		if (number_literal.length() != current - start) {
			for (size_t i = current - start; i < number_literal.length(); i++) {
				advance();
			}
		}
	}

	addToken(Token::TokenType::TOKEN_NUMBER, std::string{number_literal});
}
void Lexer::charLiteral() {
	char character = '\0';
	if (peek() != '\\') {
		character = peek();
		advance();
	} else {
		advance();
		switch (peek()) {
		case 'n': {
			character = '\n';
			break;
		}
		case '0':
			character = '\0';
			break;
		default: {
			Token errorToken{
			    Token::TokenType::TOKEN_STRING, {character}, line, column};
			errors.push_back(
			    {.category = LexerError::ErrorCategory::UnterminatedCharLiteral,
			     .token = errorToken,
			     .message =
			         std::format("Unknown escape sequence '\\{}'", peek())});
			return;
		}
		}
		advance();
	}

	if (peek() != '\'') {
		Token errorToken{
		    Token::TokenType::TOKEN_CHAR, {character}, line, column};
		errors.push_back(
		    {.category = LexerError::ErrorCategory::UnterminatedCharLiteral,
		     .token = errorToken,
		     .message = "Expected ' after char literal"});
	}
	advance();
	addToken(Token::TokenType::TOKEN_CHAR, {character});
}

void Lexer::intrinsicFunction() {
	auto startColumn = column;
	auto startLine = line;
	while (std::isalnum(peek()) || peek() == '_') {
		advance();
	}
	auto text = source.substr(start, current - start);
	addToken(Token::TokenType::TOKEN_INTRINSIC, std::string(text), startLine,
	         startColumn);
}

void Lexer::identifier() {
	while (std::isalnum(peek()) || peek() == '_') {
		advance();
	}
	auto text = source.substr(start, current - start);
	auto type = Token::fromString(text);
	addToken(type == Token::TokenType::TOKEN_ERROR
	             ? Token::TokenType::TOKEN_IDENTIFIER
	             : type,
	         std::string{type == Token::TokenType::TOKEN_ERROR ? text : ""});
}

void Lexer::comment() {
	while (!isAtEnd()) {
		if (peek() == '\n') {
			advance();
			break;
		}
		advance();
	}
}

void Lexer::multiLineComment() {
	while (!isAtEnd()) {
		if (peek() == '*' && peekNext() == '/') {
			advance();
			advance();
			break;
		}
		advance();
	}
}

char Lexer::peek() {
	if (isAtEnd()) {
		return '\0';
	}
	return source[current];
}

char Lexer::peekNext() {
	if (current + 1 >= source.length()) {
		return '\0';
	}
	return source[current + 1];
}

} // namespace ray::compiler

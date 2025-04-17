#include <cctype>
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
	line = 0;
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
		}
		break;
	}
	// not expected character
	case Token::TokenType::TOKEN_ERROR: {
		if (std::isalpha(c)) {
			identifier();
		} else if (std::isdigit(c)) {
			number();
		} else if (c == '"') {
			string();
		} else if (c == ' ' || c == '\r' || c == '\t') {
		} else if (c == '\n') {
			line++;
			column = 0;
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
	return source[current++];
};

void Lexer::addToken(Token::TokenType type) { addToken(type, ""); }
void Lexer::addToken(Token::TokenType type, std::string literal) {
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
	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') {
			line++;
			column = 0;
		}
		advance();
	}

	if (isAtEnd()) {
		std::string value = std::string{source.substr(start, current - start)};
		Token errorToken{Token::TokenType::TOKEN_STRING, value, line, column};
		errors.push_back(
		    {.category = LexerError::ErrorCategory::UnterminatedString,
		     .token = errorToken,
		     .message = "Unterminated string"});
		return;
	}

	advance();

	std::string value =
	    std::string{source.substr(start + 1, current - start - 2)};
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

	addToken(Token::TokenType::TOKEN_NUMBER,
	         std::string{source.substr(start, current - start)});
}

void Lexer::identifier() {
	while (std::isalnum(peek())) {
		advance();
	}
	auto text = source.substr(start, current - start);
	auto type = Token::fromString(text);
	addToken(type == Token::TokenType::TOKEN_ERROR
	             ? Token::TokenType::TOKEN_IDENTIFIER
	             : type,
	         std::string{type == Token::TokenType::TOKEN_ERROR ? text : ""});
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

#include <cctype>
#include <string_view>
#include <vector>

#include <ray/compiler/lexer/lexer.hpp>
#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler {

Lexer::Lexer(std::string_view source) : source(source) {}

bool Lexer::isAtEnd() const { return current >= source.length(); };

std::vector<Token> Lexer::scanTokens() {
	tokens.clear();
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

void Lexer::scanToken() {
	char c = advance();
	Token::TokenType type = Token::fromChar(c);
	switch (type) {
	case Token::TokenType::TOKEN_DOT: {
		char next = peekNext();
		if (std::isdigit(next)) {
			number();
		}
		break;
	}
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
		}
		else{
			// TODO: report an error here
			addToken(type, std::string{std::string_view(&c, 1)});
		}
		break;
	}
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
	while (peek() != '"' && isAtEnd()) {
		if (peek() == '\n') {
			line++;
			column = 0;
		}
		advance();
	}

	if (isAtEnd()) {
		// TODO: return an unterminated string with the current info
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

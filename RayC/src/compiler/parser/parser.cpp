#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/parser/parser.hpp>

namespace ray::compiler {

std::vector<ast::Statement> Parser::parse() {
	current = 0;
	try {
		std::vector<ast::Statement> statements;

		while (!isAtEnd()) {
			statements.push_back(declaration());
		}

		return statements;
	} catch (ParseException e) {
		return {};
	}
}

ast::Statement Parser::declaration() {
	try {
		if (match({Token::TokenType::TOKEN_FN})) {
			return function("function");
		}
		if (match({Token::TokenType::TOKEN_LET})) {
			return varDeclaration();
		}
		return statement();
	} catch (ParseException) {
		synchronize();
		return ast::Statement{};
	}
}

bool Parser::match(std::vector<Token::TokenType> types) {
	for (auto type : types) {
		if (check(type)) {
			advance();
			return true;
		}
	}
	return false;
}

Token Parser::advance(){
	if (!isAtEnd()) { current++; }
	return previous();
}

bool Parser::isAtEnd() const {
	return peek().type == Token::TokenType::TOKEN_EOF;
}
Token Parser::peek() const {
	return current < tokens.size() ? tokens[current]
	                               : Token{Token::TokenType::TOKEN_EOF};
}

} // namespace ray::compiler

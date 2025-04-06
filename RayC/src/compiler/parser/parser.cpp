#include <format>
#include <memory>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/parser/parser.hpp>
#include <utility>

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
ast::Function Parser::function(std::string kind) {
	Token name = consume(Token::TokenType::TOKEN_IDENTIFIER,
	                     std::format("Expect {} name.", kind));

	consume(Token::TokenType::TOKEN_LEFT_PAREN,
	        std::format("Expect '(' after {} name.", kind));
	std::vector<Token> parameters;
	if (!check(Token::TokenType::TOKEN_RIGHT_PAREN)) {
		do {
			parameters.push_back(consume(Token::TokenType::TOKEN_IDENTIFIER,
			                             "Expect parameter name."));
		} while (match({Token::TokenType::TOKEN_COMMA}));
	}
	consume(Token::TokenType::TOKEN_RIGHT_PAREN,
	        "Expect ')' after parameters.");
	consume(Token::TokenType::TOKEN_LEFT_BRACE,
	        std::format("Expect '{{' before {} body.", kind));
	auto body =
	    std::make_unique<std::vector<std::unique_ptr<ast::Statement>>>(block());
	return {std::make_unique<Token>(name),
	        std::make_unique<std::vector<Token>>(parameters), std::move(body)};
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

Token Parser::advance() {
	if (!isAtEnd()) {
		current++;
	}
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

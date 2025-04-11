#include <format>
#include <memory>
#include <utility>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/error_bag.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/parser/parser.hpp>

namespace ray::compiler {

std::vector<std::unique_ptr<ast::Statement>> Parser::parse() {
	current = 0;
	try {
		std::vector<std::unique_ptr<ast::Statement>> statements;

		while (!isAtEnd()) {
			statements.push_back(declaration());
		}

		return statements;
	} catch (ParseException e) {
		return {};
	}
}

std::unique_ptr<ast::Expression> Parser::expression() { return comma(); }

std::unique_ptr<ast::Statement> Parser::declaration() {
	try {
		if (match({Token::TokenType::TOKEN_FN})) {
			return std::make_unique<ast::Statement>(function("function"));
		}
		if (match({Token::TokenType::TOKEN_LET})) {
			return varDeclaration();
		}
		return statement();
	} catch (ParseException) {
		synchronize();
		return std::make_unique<ast::Statement>(ast::Statement{});
	}
}

std::unique_ptr<ast::Statement> Parser::varDeclaration() {
	Token name =
	    consume(Token::TokenType::TOKEN_IDENTIFIER, "Expect variable name.");

	std::unique_ptr<ast::Expression> initializer;
	if (match({Token::TokenType::TOKEN_EQUAL})) {
		initializer = expression();
	}

	consume(Token::TokenType::TOKEN_SEMICOLON,
	        "Expect ';' after variable declaration.");
	ast::Var variable{name, std::move(initializer)};
	return std::make_unique<ast::Statement>(std::move(variable));
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
	return {name, std::move(parameters), std::move(*body)};
}

std::vector<std::unique_ptr<ast::Statement>> Parser::block() {
	std::vector<std::unique_ptr<ast::Statement>> statements;

	while (!check(Token::TokenType::TOKEN_RIGHT_BRACE) && !isAtEnd()) {
		statements.push_back(declaration());
	}

	consume(Token::TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after block.");
	// add a terminal statement to the block if the last statement is not a
	// terminal statement
	if (statements.size() < 1 ||
	    dynamic_cast<ast::TerminalExpr *>(
	        statements[statements.size() - 1].get()) == nullptr) {
		statements.push_back(
		    std::make_unique<ast::Statement>(ast::TerminalExpr(nullptr)));
	}
	return statements;
}
std::unique_ptr<ast::Expression> Parser::comma() {
	auto expr = assignment();

	while (match({Token::TokenType::TOKEN_COMMA})) {
		Token op = previous();
		auto right = expression();
		expr = std::make_unique<ast::Expression>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
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
bool Parser::check(Token::TokenType type) {
	if (isAtEnd()) {
		return false;
	}
	return peek().type == type;
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
Token Parser::previous() {
	return tokens.size() < 1 ? Token{} : tokens[tokens.size() - 1];
}
Token Parser::consume(Token::TokenType type, std::string message) {
	if (check(type)) {
		return advance();
	}
	throw error(peek(), message);
}

ParseException Parser::error(Token token, std::string message) {
	ErrorBag::error(token, message);
	return ParseException();
}

void Parser::synchronize() {
	advance();

	while (!isAtEnd()) {
		if (previous().type == Token::TokenType::TOKEN_SEMICOLON) {
			return;
		}

		switch (peek().type) {
		case Token::TokenType::TOKEN_FN:
		case Token::TokenType::TOKEN_LET:
		case Token::TokenType::TOKEN_FOR:
		case Token::TokenType::TOKEN_IF:
		case Token::TokenType::TOKEN_WHILE:
		case Token::TokenType::TOKEN_RETURN:
			return;
		default: {
		}
		}

		advance();
	}
}

} // namespace ray::compiler

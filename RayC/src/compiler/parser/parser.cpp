#include <format>
#include <memory>
#include <utility>
#include <vector>

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
	} catch (ParseException &e) {
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
	} catch (ParseException &e) {
		synchronize();
		return std::make_unique<ast::Statement>(ast::Statement{});
	}
}
std::unique_ptr<ast::Statement> Parser::statement() {
	if (match({Token::TokenType::TOKEN_FOR})) {
		return forStatement();
	}
	if (match({Token::TokenType::TOKEN_IF})) {
		return ifStatement();
	}
	if (match({Token::TokenType::TOKEN_RETURN})) {
		return returnStatement();
	}
	if (match({Token::TokenType::TOKEN_CONTINUE})) {
		return continueStatement();
	}
	if (match({Token::TokenType::TOKEN_BREAK})) {
		return breakStatement();
	}
	if (match({Token::TokenType::TOKEN_WHILE})) {
		return whileStatement();
	}
	if (match({Token::TokenType::TOKEN_LEFT_BRACE})) {
		return std::make_unique<ast::Statement>(ast::Block((block())));
	}

	return expressionStatement();
}
std::unique_ptr<ast::Statement> Parser::forStatement() {
	consume(Token::TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

	std::unique_ptr<ast::Statement> initializer;
	if (!match({Token::TokenType::TOKEN_SEMICOLON})) {
		if (match({Token::TokenType::TOKEN_LET})) {
			initializer = varDeclaration();
		} else {
			initializer = expressionStatement();
		}
	}

	std::unique_ptr<ast::Expression> condition;
	if (!check(Token::TokenType::TOKEN_SEMICOLON)) {
		condition = expression();
	}
	consume(Token::TokenType::TOKEN_SEMICOLON,
	        "Expect ';' after loop condition.");

	std::unique_ptr<ast::Expression> increment;
	if (!check(Token::TokenType::TOKEN_RIGHT_PAREN)) {
		increment = expression();
	}
	consume(Token::TokenType::TOKEN_RIGHT_PAREN,
	        "Expect ')' after for clauses.");

	std::unique_ptr<ast::Statement> body = statement();

	if (increment) {
		std::vector<std::unique_ptr<ast::Statement>> bodyStatements;
		bodyStatements.push_back(std::move(body));
		bodyStatements.push_back(std::make_unique<ast::Statement>(
		    ast::ExpressionStmt(std::move(increment))));
		body = std::make_unique<ast::Statement>(
		    ast::Block(std::move(bodyStatements)));
	}

	if (!condition) {
		condition = std::make_unique<ast::Expression>(ast::Literal(true));
	}
	body = std::make_unique<ast::Statement>(
	    ast::While(std::move(condition), std::move(body)));

	if (initializer) {
		std::vector<std::unique_ptr<ast::Statement>> bodyStatements;
		bodyStatements.push_back(std::move(initializer));
		bodyStatements.push_back(std::move(body));
		body = std::make_unique<ast::Statement>(
		    ast::Block(std::move(bodyStatements)));
	}

	return body;
}
std::unique_ptr<ast::Statement> Parser::ifStatement() {
	consume(Token::TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	auto condition = expression();
	consume(Token::TokenType::TOKEN_RIGHT_PAREN,
	        "Expect ')' after if condition.");

	auto thenBranch = statement();
	std::unique_ptr<ast::Statement> elseBranch = nullptr;
	if (match({Token::TokenType::TOKEN_ELSE})) {
		elseBranch = statement();
	}

	return std::make_unique<ast::Statement>(ast::If(
	    std::move(condition), std::move(thenBranch), std::move(elseBranch)));
}
std::unique_ptr<ast::Statement> Parser::returnStatement() {
	Token keyword = previous();
	std::unique_ptr<ast::Expression> value;
	if (!check(Token::TokenType::TOKEN_SEMICOLON)) {
		value = expression();
	}
	consume(Token::TokenType::TOKEN_SEMICOLON,
	        "Expect ';' after return value.");
	return std::make_unique<ast::Statement>(
	    ast::Jump(keyword, std::move(value)));
}
std::unique_ptr<ast::Statement> Parser::continueStatement() {
	Token keyword = previous();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after continue.");
	return std::make_unique<ast::Statement>(ast::Jump(keyword, nullptr));
}
std::unique_ptr<ast::Statement> Parser::breakStatement() {
	Token keyword = previous();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after break.");
	return std::make_unique<ast::Statement>(ast::Jump(keyword, nullptr));
}
std::unique_ptr<ast::Statement> Parser::whileStatement() {
	consume(Token::TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
	auto condition = expression();
	consume(Token::TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	auto body = statement();

	return std::make_unique<ast::Statement>(
	    ast::While(std::move(condition), std::move(body)));
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
std::unique_ptr<ast::Statement> Parser::expressionStatement() {
	auto expr = expression();
	if (match({Token::TokenType::TOKEN_SEMICOLON})) {
		return std::make_unique<ast::Statement>(
		    ast::ExpressionStmt(std::move(expr)));
	}
	return std::make_unique<ast::Statement>(ast::TerminalExpr(std::move(expr)));
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
std::unique_ptr<ast::Expression> Parser::assignment() {
	auto expr = orExpression();

	if (match({Token::TokenType::TOKEN_EQUAL})) {
		Token equals = previous();
		auto value = assignment();
		if (auto variable = dynamic_cast<ast::Variable *>(expr.get())) {
			Token name = variable->name;
			return std::make_unique<ast::Expression>(
			    ast::Assign(name, std::move(value)));
		} else if (auto get = dynamic_cast<ast::Get *>(expr.get())) {
			return std::make_unique<ast::Expression>(
			    ast::Set(std::move(get->object), get->name, std::move(value)));
		}
		error(equals, "Invalid assignment target.");
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::orExpression() {
	auto expr = andExpression();

	while (match({Token::TokenType::TOKEN_PIPE_PIPE})) {
		Token op = previous();
		auto right = andExpression();
		expr = std::make_unique<ast::Expression>(
		    ast::Logical(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::andExpression() {
	auto expr = equalityExpression();

	while (match({Token::TokenType::TOKEN_AMPERSAND_AMPERSAND})) {
		Token op = previous();
		auto right = equalityExpression();
		expr = std::make_unique<ast::Expression>(
		    ast::Logical(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::equalityExpression() {
	auto expr = comparisonExpression();

	while (match({Token::TokenType::TOKEN_BANG_EQUAL,
	              Token::TokenType::TOKEN_EQUAL_EQUAL})) {
		Token op = previous();
		auto right = comparisonExpression();
		expr = std::make_unique<ast::Expression>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::comparisonExpression() {
	auto expr = terminalExpression();

	while (match(
	    {Token::TokenType::TOKEN_GREAT, Token::TokenType::TOKEN_GREAT_EQUAL,
	     Token::TokenType::TOKEN_LESS, Token::TokenType::TOKEN_LESS_EQUAL})) {
		Token op = previous();
		auto right = terminalExpression();
		expr = std::make_unique<ast::Expression>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::terminalExpression() {
	auto expr = factorExpression();

	while (
	    match({Token::TokenType::TOKEN_MINUS, Token::TokenType::TOKEN_PLUS})) {
		Token op = previous();
		auto right = factorExpression();
		expr = std::make_unique<ast::Expression>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::factorExpression() {
	auto expr = unaryExpression();

	while (
	    match({Token::TokenType::TOKEN_SLASH, Token::TokenType::TOKEN_STAR})) {
		Token op = previous();
		auto right = unaryExpression();
		expr = std::make_unique<ast::Expression>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::unaryExpression() {
	if (match({Token::TokenType::TOKEN_BANG, Token::TokenType::TOKEN_MINUS})) {
		auto op = previous();
		auto right = unaryExpression();
		return std::make_unique<ast::Expression>(
		    ast::Unary(op, std::move(right)));
	}
	return call();
}
std::unique_ptr<ast::Expression>
Parser::finishCall(std::unique_ptr<ast::Expression> callee) {
	std::vector<std::unique_ptr<ast::Expression>> arguments;
	if (!check(Token::TokenType::TOKEN_RIGHT_PAREN)) {
		do {
			auto argument = expression();
			ast::Binary *binaryArg =
			    dynamic_cast<ast::Binary *>(argument.get());
			if (binaryArg &&
			    binaryArg->op.type == Token::TokenType::TOKEN_COMMA) {
				do {
					arguments.push_back(std::move(binaryArg->left));
					argument = std::move(binaryArg->right);
					binaryArg = dynamic_cast<ast::Binary *>(argument.get());
				} while (binaryArg &&
				         binaryArg->op.type == Token::TokenType::TOKEN_COMMA);
			}
			arguments.push_back(std::move(argument));
		} while (match({Token::TokenType::TOKEN_COMMA}));
	}

	auto paren = consume(Token::TokenType::TOKEN_RIGHT_PAREN,
	                     "Expect ')' after arguments.");
	return std::make_unique<ast::Expression>(
	    ast::Call(std::move(callee), paren, std::move(arguments)));
}
std::unique_ptr<ast::Expression> Parser::call() {
	auto expr = primaryExpresion();

	while (true) {
		if (match({Token::TokenType::TOKEN_LEFT_PAREN})) {
			expr = finishCall(std::move(expr));
		} else {
			break;
		}
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::primaryExpresion() {
	if (match({Token::TokenType::TOKEN_FALSE})) {
		return std::make_unique<ast::Expression>(ast::Literal(false));
	}
	if (match({Token::TokenType::TOKEN_TRUE})) {
		return std::make_unique<ast::Expression>(ast::Literal(true));
	}

	if (match(
	        {Token::TokenType::TOKEN_NUMBER, Token::TokenType::TOKEN_STRING})) {
		return std::make_unique<ast::Expression>(
		    ast::Literal(previous().lexeme));
	}

	if (match({Token::TokenType::TOKEN_IDENTIFIER})) {
		return std::make_unique<ast::Expression>(ast::Variable(previous()));
	}

	if (match({Token::TokenType::TOKEN_LEFT_PAREN})) {
		auto expr = expression();
		consume(Token::TokenType::TOKEN_RIGHT_PAREN,
		        "Expect ')' after expression.");
		return std::make_unique<ast::Expression>(
		    ast::Grouping(std::move(expr)));
	}

	throw error(peek(), "Expect expression.");
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

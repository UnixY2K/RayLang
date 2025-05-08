#include <format>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/error_bag.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/parser/parser.hpp>

namespace ray::compiler {

Parser::Parser(std::string filepath, std::vector<Token> tokens)
    : errorBag(filepath), tokens(tokens) {}

std::vector<std::unique_ptr<ast::Statement>> Parser::parse() {
	current = 0;
	try {
		std::vector<std::unique_ptr<ast::Statement>> statements;

		while (!isAtEnd()) {
			auto result = declaration();
			if (result.has_value()) {
				statements.push_back(std::move(result.value()));
			}
		}

		return statements;
	} catch (ParseException &e) {
		return {};
	}
}

bool Parser::failed() const { return errorBag.failed(); }

std::unique_ptr<ast::Expression> Parser::expression() { return comma(); }
std::optional<std::unique_ptr<ast::Statement>> Parser::declaration() {
	try {
		Token pubToken;
		if (match({Token::TokenType::TOKEN_PUB})) {
			pubToken = previous();
		}
		if (match({Token::TokenType::TOKEN_STRUCT})) {
			return structDeclaration(pubToken.type ==
			                         Token::TokenType::TOKEN_PUB);
		}
		if (match({Token::TokenType::TOKEN_FN})) {
			return std::make_unique<ast::Function>(function(
			    "function", pubToken.type == Token::TokenType::TOKEN_PUB));
		}
		if (match({Token::TokenType::TOKEN_LET})) {
			if (pubToken.type == Token::TokenType::TOKEN_PUB) {
				error(pubToken,
				      "pub token cannot be in a variable declaration");
			}
			return std::make_unique<ast::Var>(varDeclaration("variable"));
		}
		return statement();
	} catch (ParseException &e) {
		synchronize();
		return std::nullopt;
	}
}
std::unique_ptr<ast::Statement>
Parser::structDeclaration(bool publicVisibility) {
	Token name =
	    consume(Token::TokenType::TOKEN_IDENTIFIER, "Expect struct name.");
	std::vector<ast::Var> members;
	std::vector<bool> memberVisibility;
	bool structDeclaration = match({Token::TokenType::TOKEN_SEMICOLON});

	if (!structDeclaration) {
		consume(Token::TokenType::TOKEN_LEFT_BRACE,
		        "Expect '{' before struct body.");
		while (!check(Token::TokenType::TOKEN_RIGHT_BRACE) && !isAtEnd()) {
			Token pubToken;
			if (match({Token::TokenType::TOKEN_PUB})) {
				pubToken = previous();
			}
			members.push_back(varDeclaration("member"));
		}
		consume(Token::TokenType::TOKEN_RIGHT_BRACE,
		        "Expect '}' after struct body.");
	}
	return std::make_unique<ast::Struct>(
	    ast::Struct{name, publicVisibility, structDeclaration,
	                std::move(members), memberVisibility});
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
		return std::make_unique<ast::Block>(ast::Block((block())));
	}

	return expressionStatement();
}
std::unique_ptr<ast::Statement> Parser::forStatement() {
	consume(Token::TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

	std::unique_ptr<ast::Statement> initializer;
	if (!match({Token::TokenType::TOKEN_SEMICOLON})) {
		if (match({Token::TokenType::TOKEN_LET})) {
			initializer =
			    std::make_unique<ast::Var>(varDeclaration("variable"));
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
		bodyStatements.push_back(std::make_unique<ast::ExpressionStmt>(
		    ast::ExpressionStmt(std::move(increment))));
		body =
		    std::make_unique<ast::Block>(ast::Block(std::move(bodyStatements)));
	}

	if (!condition) {
		Token token = Token(Token::TokenType::TOKEN_TRUE, "", 0, 0);
		condition = std::make_unique<ast::Literal>(ast::Literal(token, "true"));
	}
	body = std::make_unique<ast::While>(
	    ast::While(std::move(condition), std::move(body)));

	if (initializer) {
		std::vector<std::unique_ptr<ast::Statement>> bodyStatements;
		bodyStatements.push_back(std::move(initializer));
		bodyStatements.push_back(std::move(body));
		body =
		    std::make_unique<ast::Block>(ast::Block(std::move(bodyStatements)));
	}

	return body;
}
std::unique_ptr<ast::Statement> Parser::ifStatement() {
	auto condition = expression();

	auto thenBranch = statement();
	std::optional<std::unique_ptr<ast::Statement>> elseBranch = std::nullopt;
	if (match({Token::TokenType::TOKEN_ELSE})) {
		elseBranch = statement();
	}

	return std::make_unique<ast::If>(ast::If(
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
	return std::make_unique<ast::Jump>(ast::Jump(keyword, std::move(value)));
}
std::unique_ptr<ast::Statement> Parser::continueStatement() {
	Token keyword = previous();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after continue.");
	return std::make_unique<ast::Jump>(ast::Jump(keyword, nullptr));
}
std::unique_ptr<ast::Statement> Parser::breakStatement() {
	Token keyword = previous();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after break.");
	return std::make_unique<ast::Jump>(ast::Jump(keyword, nullptr));
}
std::unique_ptr<ast::Statement> Parser::whileStatement() {
	auto condition = expression();

	auto body = statement();

	return std::make_unique<ast::While>(
	    ast::While(std::move(condition), std::move(body)));
}
ast::Var Parser::varDeclaration(std::string kind) {
	bool is_mutable = false;
	if (match({Token::TokenType::TOKEN_MUT})) {
		is_mutable = true;
	}
	Token name = consume(Token::TokenType::TOKEN_IDENTIFIER,
	                     std::format("Expect {} name.", kind));

	ast::Type type =
	    ast::Type{Token{Token::TokenType::TOKEN_UNINITIALIZED}, true, false};
	if (match({Token::TokenType::TOKEN_COLON})) {
		// array type
		type = typeExpression();
	}

	std::optional<std::unique_ptr<ast::Expression>> initializer = std::nullopt;
	if (match({Token::TokenType::TOKEN_EQUAL})) {
		initializer = expression();
	}

	consume(Token::TokenType::TOKEN_SEMICOLON,
	        std::format("Expect ';' after {} declaration.", kind));
	ast::Var variable{name, type, is_mutable, std::move(initializer)};
	return ast::Var(std::move(variable));
}
std::unique_ptr<ast::Statement> Parser::expressionStatement() {
	auto expr = expression();
	if (match({Token::TokenType::TOKEN_SEMICOLON})) {
		return std::make_unique<ast::ExpressionStmt>(
		    ast::ExpressionStmt(std::move(expr)));
	}
	return std::make_unique<ast::TerminalExpr>(
	    ast::TerminalExpr(std::move(expr)));
}
ast::Function Parser::function(std::string kind, bool publicVisiblity) {
	Token name = consume(Token::TokenType::TOKEN_IDENTIFIER,
	                     std::format("Expect {} name.", kind));

	consume(Token::TokenType::TOKEN_LEFT_PAREN,
	        std::format("Expect '(' after {} name.", kind));
	std::vector<ast::Parameter> parameters;
	if (!check(Token::TokenType::TOKEN_RIGHT_PAREN)) {
		do {
			auto parameterName = consume(Token::TokenType::TOKEN_IDENTIFIER,
			                             "Expect parameter name.");
			consume(Token::TokenType::TOKEN_COLON,
			        "Expect ':' after parameter name.");
			ast::Type parameterType = typeExpression();
			auto parameter = ast::Parameter(parameterName, parameterType);

			parameters.push_back(parameter);
		} while (match({Token::TokenType::TOKEN_COMMA}));
	}
	consume(Token::TokenType::TOKEN_RIGHT_PAREN,
	        "Expect ')' after parameters.");

	auto previousToken = previous();

	ast::Type returnType(
	    types::makeUnitTypeToken(previous().line, previous().column), true,
	    false);

	if (match({Token::TokenType::TOKEN_ARROW})) {
		returnType = typeExpression();
	}

	std::optional<ast::Block> body = std::nullopt;
	if (!match({Token::TokenType::TOKEN_SEMICOLON})) {
		consume(Token::TokenType::TOKEN_LEFT_BRACE,
		        std::format("Expect '{{' before {} body.", kind));
		body = {block()};
	}
	return {name, publicVisiblity, parameters, std::move(body), returnType};
}
std::vector<std::unique_ptr<ast::Statement>> Parser::block() {
	std::vector<std::unique_ptr<ast::Statement>> statements;

	while (!check(Token::TokenType::TOKEN_RIGHT_BRACE) && !isAtEnd()) {
		auto result = declaration();
		if (result.has_value()) {
			statements.push_back(std::move(result.value()));
		}
	}

	consume(Token::TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after block.");
	// add a terminal statement to the block if the last statement is not a
	// terminal statement
	if (statements.size() < 1 ||
	    dynamic_cast<ast::TerminalExpr *>(
	        statements[statements.size() - 1].get()) == nullptr) {
		statements.push_back(
		    std::make_unique<ast::TerminalExpr>(ast::TerminalExpr({})));
	}
	return statements;
}
std::unique_ptr<ast::Expression> Parser::comma() {
	auto expr = assignment();

	while (match({Token::TokenType::TOKEN_COMMA})) {
		Token op = previous();
		auto right = expression();
		expr = std::make_unique<ast::Binary>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::assignment() {
	auto lhs_expr = orExpression();

	if (match({
	        Token::TokenType::TOKEN_EQUAL,             // =
	        Token::TokenType::TOKEN_PLUS_EQUAL,        // +=
	        Token::TokenType::TOKEN_MINUS_EQUAL,       // -=
	        Token::TokenType::TOKEN_STAR_EQUAL,        // *=
	        Token::TokenType::TOKEN_SLASH_EQUAL,       // /=
	        Token::TokenType::TOKEN_PERCENT_EQUAL,     // *=
	        Token::TokenType::TOKEN_AMPERSAND_EQUAL,   // &=
	        Token::TokenType::TOKEN_PIPE_EQUAL,        // |=
	        Token::TokenType::TOKEN_CARET_EQUAL,       // ^=
	        Token::TokenType::TOKEN_LESS_LESS_EQUAL,   // <<=
	        Token::TokenType::TOKEN_GREAT_GREAT_EQUAL, // >>=
	    })) {
		Token assignmentOp = previous();
		auto rhs_expr = assignment();
		if (dynamic_cast<ast::Variable *>(lhs_expr.get())) {
			return std::make_unique<ast::Assign>(ast::Assign(
			    std::move(lhs_expr), assignmentOp, std::move(rhs_expr)));
		} else if (auto get = dynamic_cast<ast::Get *>(lhs_expr.get())) {
			return std::make_unique<ast::Set>(ast::Set{std::move(get->object),
			                                           get->name, assignmentOp,
			                                           std::move(rhs_expr)});
		} else if (dynamic_cast<ast::ArrayAccess *>(lhs_expr.get())) {
			return std::make_unique<ast::Assign>(ast::Assign(
			    std::move(lhs_expr), assignmentOp, std::move(rhs_expr)));
		}
		error(assignmentOp, std::format("Invalid assignment target '{}'.",
		                                lhs_expr->variantName()));
	}

	return lhs_expr;
}
std::unique_ptr<ast::Expression> Parser::orExpression() {
	auto expr = andExpression();

	while (match({Token::TokenType::TOKEN_PIPE_PIPE})) {
		Token op = previous();
		auto right = andExpression();
		expr = std::make_unique<ast::Logical>(
		    ast::Logical(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::andExpression() {
	auto expr = equalityExpression();

	while (match({Token::TokenType::TOKEN_AMPERSAND_AMPERSAND})) {
		Token op = previous();
		auto right = equalityExpression();
		expr = std::make_unique<ast::Logical>(
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
		expr = std::make_unique<ast::Binary>(
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
		expr = std::make_unique<ast::Binary>(
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
		expr = std::make_unique<ast::Binary>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::factorExpression() {
	auto expr = unaryExpression();

	while (match({Token::TokenType::TOKEN_SLASH, Token::TokenType::TOKEN_STAR,
	              Token::TokenType::TOKEN_PERCENT})) {
		Token op = previous();
		auto right = unaryExpression();
		expr = std::make_unique<ast::Binary>(
		    ast::Binary(std::move(expr), op, std::move(right)));
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::unaryExpression() {
	if (match({Token::TokenType::TOKEN_BANG, Token::TokenType::TOKEN_MINUS,
	           Token::TokenType::TOKEN_PLUS_PLUS,
	           Token::TokenType::TOKEN_MINUS_MINUS})) {
		auto op = previous();
		auto right = unaryExpression();
		return std::make_unique<ast::Unary>(
		    ast::Unary(op, true, std::move(right)));
	}
	auto expr = call();
	if (match({Token::TokenType::TOKEN_AS})) {
		auto type = typeExpression();
		expr = std::make_unique<ast::Cast>(ast::Cast{std::move(expr), type});
	}
	return expr;
}
ast::Type Parser::typeExpression() {
	bool is_mutable = match({Token::TokenType::TOKEN_MUT});
	if (match({Token::TokenType::TOKEN_LEFT_SQUARE_BRACE})) {
		auto arrayStartToken = previous();
		auto arrayTypeToken = consume(Token::TokenType::TOKEN_IDENTIFIER,
		                              "Expect type signature");
		consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after type.");
		consume(Token::TokenType::TOKEN_RIGHT_SQUARE_BRACE,
		        "Expect ']' after array type.");
		return ast::Type{Token{Token::TokenType::TOKEN_LEFT_SQUARE_BRACE,
		                       std::format("[{}]", arrayStartToken.lexeme +
		                                               arrayTypeToken.lexeme),
		                       arrayStartToken.line, arrayStartToken.column},
		                 !is_mutable, true};
	}
	auto typeToken =
	    consume(Token::TokenType::TOKEN_IDENTIFIER, "Expect type signature");
	bool isPointer = false;
	while (match({Token::TokenType::TOKEN_STAR})) {
		isPointer = true;
		typeToken.lexeme += Token::glyph(previous().type);
	}
	return ast::Type{typeToken, !is_mutable, isPointer};
}

std::unique_ptr<ast::Expression>
Parser::finishArrayAccess(std::unique_ptr<ast::Expression> callee) {
	auto index = expression();
	auto paren = consume(Token::TokenType::TOKEN_RIGHT_SQUARE_BRACE,
	                     "Expect ']' after array access");
	return std::make_unique<ast::ArrayAccess>(
	    ast::ArrayAccess{std::move(callee), std::move(index)});
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
	return std::make_unique<ast::Call>(
	    ast::Call(std::move(callee), paren, std::move(arguments)));
}
std::unique_ptr<ast::Expression> Parser::call() {
	auto expr = primaryExpresion();
	while (true) {
		if (match({Token::TokenType::TOKEN_LEFT_PAREN})) {
			expr = finishCall(std::move(expr));
		} else if (match({Token::TokenType::TOKEN_DOT})) {
			Token name = consume(Token::TokenType::TOKEN_IDENTIFIER,
			                     "Expect property name after '.'.");
			expr = std::make_unique<ast::Get>(ast::Get{std::move(expr), name});
		} else if (match({Token::TokenType::TOKEN_LEFT_SQUARE_BRACE})) {
			expr = finishArrayAccess(std::move(expr));
		} else if (match({Token::TokenType::TOKEN_PLUS_PLUS,
		                  Token::TokenType::TOKEN_MINUS_MINUS})) {
			expr = std::make_unique<ast::Unary>(
			    ast::Unary{previous(), false, std::move(expr)});
		} else {
			break;
		}
	}

	return expr;
}

std::unique_ptr<ast::Expression> Parser::primaryExpresion() {
	Token kind = peek();
	if (match({Token::TokenType::TOKEN_FALSE})) {
		return std::make_unique<ast::Literal>(ast::Literal(kind, "false"));
	}
	if (match({Token::TokenType::TOKEN_TRUE})) {
		return std::make_unique<ast::Literal>(ast::Literal(kind, "true"));
	}

	if (match({Token::TokenType::TOKEN_NUMBER, Token::TokenType::TOKEN_STRING,
	           Token::TokenType::TOKEN_CHAR})) {
		return std::make_unique<ast::Literal>(
		    ast::Literal(kind, previous().lexeme));
	}

	if (match({Token::TokenType::TOKEN_IDENTIFIER})) {
		return std::make_unique<ast::Variable>(ast::Variable(previous()));
	}

	if (match({Token::TokenType::TOKEN_LEFT_PAREN})) {
		auto expr = expression();
		consume(Token::TokenType::TOKEN_RIGHT_PAREN,
		        "Expect ')' after expression.");
		return std::make_unique<ast::Grouping>(ast::Grouping(std::move(expr)));
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
bool Parser::check(Token::TokenType type) { return peek().type == type; }

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
	return tokens.size() < 1 ? Token{} : tokens[current - 1];
}
Token Parser::consume(Token::TokenType type, std::string message) {
	if (check(type)) {
		return advance();
	}
	throw error(peek(), message);
}

ParseException Parser::error(Token token, std::string message) {
	errorBag.error(token, message);
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

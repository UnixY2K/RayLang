#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ray/cli/terminal.hpp>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/intrinsic.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/parser/parser.hpp>

namespace ray::compiler {

using namespace terminal::literals;

Parser::Parser(std::string filepath, std::vector<Token> tokens)
    : errorBag(filepath), tokens(tokens) {}

std::vector<std::unique_ptr<ast::Statement>> Parser::parse() {
	current = 0;
	try {
		std::vector<std::unique_ptr<ast::Statement>> statements{};
		while (!isAtEnd()) {
			auto stmts = CompilerDirective();
			if (stmts.has_value()) {
				statements.push_back(std::move(*stmts));
			}
		}
		return statements;
	} catch (ParseException &e) {
		return {};
	}
}

bool Parser::failed() const { return errorBag.failed(); }
const std::vector<std::string> Parser::getErrors() const {
	return errorBag.getErrors();
}

std::optional<std::unique_ptr<ast::Statement>> Parser::CompilerDirective() {
	if (match({Token::TokenType::TOKEN_POUND})) {
		consume(Token::TokenType::TOKEN_LEFT_SQUARE_BRACE,
		        "expected '[' before compiler directive");
		auto name = consume(Token::TokenType::TOKEN_IDENTIFIER,
		                    "expected compiler directive name");
		consume(Token::TokenType::TOKEN_LEFT_PAREN,
		        "expected '(' before compiler directive attributes");
		ast::CompDirectiveAttr attributes;
		do {
			auto attributeToken = consume(Token::TokenType::TOKEN_IDENTIFIER,
			                              "Expect attribute name.");
			std::string attributeName = std::string(attributeToken.getLexeme());
			if (attributes.contains(std::string(attributeToken.getLexeme()))) {
				error(attributeToken,
				      std::format(
				          "'{}' is a duplicated compiler directive attribute",
				          attributeToken.getLexeme()));
			}
			std::string attributeValue;
			if (match({Token::TokenType::TOKEN_EQUAL})) {
				auto valueToken = consume(Token::TokenType::TOKEN_STRING,
				                          "expected string literal value for "
				                          "compiler directive attribute");
				attributeValue = valueToken.getLexeme();
			}
			attributes[attributeName] = attributeValue;
		} while (match({Token::TokenType::TOKEN_COMMA}));
		consume(Token::TokenType::TOKEN_RIGHT_PAREN,
		        "expected ')' after compiler directive attributes");
		auto end = consume(Token::TokenType::TOKEN_RIGHT_SQUARE_BRACE,
		                   "expected ']' after compiler directive");
		// all compiler directives expect a child statement
		auto stmt = declaration();
		return std::make_unique<ast::CompDirective>(
		    name, attributes, stmt ? std::move(*stmt) : nullptr, name);
	}
	return declaration();
}
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
	return std::make_unique<ast::Struct>(ast::Struct{
	    name,
	    publicVisibility,
	    structDeclaration,
	    std::move(members),
	    memberVisibility,
	    name,
	});
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
		return std::make_unique<ast::Block>(ast::Block({
		    block(),
		    previous(),
		}));
	}

	return expressionStatement();
}
std::unique_ptr<ast::Statement> Parser::forStatement() {
	auto forExprToken = previous();
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
		auto incrementToken = increment->getToken();
		bodyStatements.push_back(
		    std::make_unique<ast::ExpressionStmt>(ast::ExpressionStmt{
		        std::move(increment),
		        incrementToken,
		    }));
		body = std::make_unique<ast::Block>(ast::Block{
		    std::move(bodyStatements),
		    incrementToken,
		});
	}

	if (!condition) {
		Token token = Token(Token::TokenType::TOKEN_TRUE, "", 0, 0);
		condition = std::make_unique<ast::Literal>(ast::Literal{
		    token,
		    "true",
		    token,
		});
	}
	body = std::make_unique<ast::While>(ast::While{
	    std::move(condition),
	    std::move(body),
	    forExprToken,
	});

	if (initializer) {
		std::vector<std::unique_ptr<ast::Statement>> bodyStatements;
		auto initializerToken = initializer->getToken();
		bodyStatements.push_back(std::move(initializer));
		bodyStatements.push_back(std::move(body));
		body = std::make_unique<ast::Block>(ast::Block{
		    std::move(bodyStatements),
		    initializerToken,
		});
	}

	return body;
}
std::unique_ptr<ast::Statement> Parser::ifStatement() {
	auto ifToken = previous();
	auto condition = expression();

	auto thenBranch = statement();
	std::optional<std::unique_ptr<ast::Statement>> elseBranch = std::nullopt;
	if (match({Token::TokenType::TOKEN_ELSE})) {
		elseBranch = statement();
	}

	return std::make_unique<ast::If>(ast::If{
	    std::move(condition),
	    std::move(thenBranch),
	    std::move(elseBranch),
	    ifToken,
	});
}
std::unique_ptr<ast::Statement> Parser::returnStatement() {
	Token keyword = previous();
	std::unique_ptr<ast::Expression> value;
	if (!check(Token::TokenType::TOKEN_SEMICOLON)) {
		value = expression();
	}
	consume(Token::TokenType::TOKEN_SEMICOLON,
	        "Expect ';' after return value.");
	return std::make_unique<ast::Jump>(ast::Jump{
	    keyword,
	    std::move(value),
	    keyword,
	});
}
std::unique_ptr<ast::Statement> Parser::continueStatement() {
	Token keyword = previous();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after continue.");
	return std::make_unique<ast::Jump>(ast::Jump{
	    keyword,
	    nullptr,
	    keyword,
	});
}
std::unique_ptr<ast::Statement> Parser::breakStatement() {
	Token keyword = previous();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after break.");
	return std::make_unique<ast::Jump>(ast::Jump{
	    keyword,
	    nullptr,
	    keyword,
	});
}
std::unique_ptr<ast::Statement> Parser::whileStatement() {
	auto token = previous();
	auto condition = expression();

	auto body = statement();

	return std::make_unique<ast::While>(ast::While{
	    std::move(condition),
	    std::move(body),
	    token,
	});
}
ast::Var Parser::varDeclaration(std::string kind) {
	bool is_mutable = match({Token::TokenType::TOKEN_MUT});
	Token name = consume(Token::TokenType::TOKEN_IDENTIFIER,
	                     std::format("Expect {} name.", kind));
	auto typeToken = Token{
	    Token::TokenType::TOKEN_UNINITIALIZED,
	    std::string(Token::glyph(Token::TokenType::TOKEN_UNINITIALIZED)),
	    name.line,
	    name.column,
	};
	ast::Type type = ast::Type{
	    typeToken, false, false, {}, typeToken,
	};
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
	return {
	    name, std::move(type), is_mutable, std::move(initializer), name,
	};
}
std::unique_ptr<ast::Statement> Parser::expressionStatement() {
	auto expr = expression();
	auto exprToken = expr->getToken();
	if (match({Token::TokenType::TOKEN_SEMICOLON})) {
		return std::make_unique<ast::ExpressionStmt>(
		    ast::ExpressionStmt(std::move(expr), exprToken));
	}
	return std::make_unique<ast::TerminalExpr>(
	    ast::TerminalExpr(std::move(expr), exprToken));
}
ast::Function Parser::function(std::string kind, bool publicVisiblity) {

	Token name = consume(Token::TokenType::TOKEN_IDENTIFIER,
	                     std::format("Expect {} name.", kind));

	consume(Token::TokenType::TOKEN_LEFT_PAREN,
	        std::format("Expect '(' after {} name.", kind));
	std::vector<ast::Parameter> parameters;
	if (!check(Token::TokenType::TOKEN_RIGHT_PAREN)) {
		do {
			auto attributeToken = consume(Token::TokenType::TOKEN_IDENTIFIER,
			                              "Expect parameter name.");
			consume(Token::TokenType::TOKEN_COLON,
			        "Expect ':' after parameter name.");
			ast::Type parameterType = typeExpression();
			auto parameter = ast::Parameter{
			    attributeToken,
			    std::move(parameterType),
			    attributeToken,
			};

			parameters.push_back(std::move(parameter));
		} while (match({Token::TokenType::TOKEN_COMMA}));
	}
	consume(Token::TokenType::TOKEN_RIGHT_PAREN,
	        "Expect ')' after parameters.");

	auto previousToken = previous();

	size_t newColumn = previous().column + previous().getLexeme().size();
	auto returnToken = types::makeUnitTypeToken(previous().line, newColumn);
	ast::Type returnType{
	    returnToken, false, false, {}, returnToken,
	};

	if (match({Token::TokenType::TOKEN_ARROW})) {
		returnType = typeExpression();
	}

	std::optional<ast::Block> body = std::nullopt;
	if (!match({Token::TokenType::TOKEN_SEMICOLON})) {
		auto token = consume(Token::TokenType::TOKEN_LEFT_BRACE,
		                     std::format("Expect '{{' before {} body.", kind));
		body = {
		    block(),
		    token,
		};
	}
	return {
	    name,
	    publicVisiblity,
	    std::move(parameters),
	    std::move(body),
	    std::move(returnType),
	    name,
	};
}
std::vector<std::unique_ptr<ast::Statement>> Parser::block() {
	std::vector<std::unique_ptr<ast::Statement>> statements;

	while (!check(Token::TokenType::TOKEN_RIGHT_BRACE) && !isAtEnd()) {
		auto result = declaration();
		if (result.has_value()) {
			statements.push_back(std::move(result.value()));
		}
	}

	auto token =
	    consume(Token::TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after block.");
	// add a terminal statement to the block if the last statement is not a
	// terminal statement
	if (statements.size() < 1 ||
	    dynamic_cast<ast::TerminalExpr *>(
	        statements[statements.size() - 1].get()) == nullptr) {
	}
	return statements;
}
std::unique_ptr<ast::Expression> Parser::comma() {
	auto expr = assignment();

	while (match({Token::TokenType::TOKEN_COMMA})) {
		Token op = previous();
		auto right = expression();
		expr = std::make_unique<ast::Binary>(
		    ast::Binary(std::move(expr), op, std::move(right), op));
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
			return std::make_unique<ast::Assign>(ast::Assign{
			    std::move(lhs_expr),
			    assignmentOp,
			    std::move(rhs_expr),
			    assignmentOp,
			});
		} else if (auto get = dynamic_cast<ast::Get *>(lhs_expr.get())) {
			return std::make_unique<ast::Set>(ast::Set{
			    std::move(get->object),
			    get->name,
			    assignmentOp,
			    std::move(rhs_expr),
			    assignmentOp,
			});
		} else if (dynamic_cast<ast::ArrayAccess *>(lhs_expr.get())) {
			return std::make_unique<ast::Assign>(ast::Assign{
			    std::move(lhs_expr),
			    assignmentOp,
			    std::move(rhs_expr),
			    assignmentOp,
			});
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
		expr = std::make_unique<ast::Logical>(ast::Logical{
		    std::move(expr),
		    op,
		    std::move(right),
		    op,
		});
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::andExpression() {
	auto expr = equalityExpression();

	while (match({Token::TokenType::TOKEN_AMPERSAND_AMPERSAND})) {
		Token op = previous();
		auto right = equalityExpression();
		expr = std::make_unique<ast::Logical>(ast::Logical{
		    std::move(expr),
		    op,
		    std::move(right),
		    op,
		});
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::equalityExpression() {
	auto expr = comparisonExpression();

	while (match({Token::TokenType::TOKEN_BANG_EQUAL,
	              Token::TokenType::TOKEN_EQUAL_EQUAL})) {
		Token op = previous();
		auto right = comparisonExpression();
		expr = std::make_unique<ast::Binary>(ast::Binary{
		    std::move(expr),
		    op,
		    std::move(right),
		    op,
		});
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
		expr = std::make_unique<ast::Binary>(ast::Binary{
		    std::move(expr),
		    op,
		    std::move(right),
		    op,
		});
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::terminalExpression() {
	auto expr = factorExpression();

	while (
	    match({Token::TokenType::TOKEN_MINUS, Token::TokenType::TOKEN_PLUS})) {
		Token op = previous();
		auto right = factorExpression();
		expr = std::make_unique<ast::Binary>(ast::Binary{
		    std::move(expr),
		    op,
		    std::move(right),
		    op,
		});
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::factorExpression() {
	auto expr = unaryExpression();

	while (match({Token::TokenType::TOKEN_SLASH, Token::TokenType::TOKEN_STAR,
	              Token::TokenType::TOKEN_PERCENT})) {
		Token op = previous();
		auto right = unaryExpression();
		expr = std::make_unique<ast::Binary>(ast::Binary{
		    std::move(expr),
		    op,
		    std::move(right),
		    op,
		});
	}

	return expr;
}
std::unique_ptr<ast::Expression> Parser::unaryExpression() {
	if (match({Token::TokenType::TOKEN_BANG, Token::TokenType::TOKEN_MINUS,
	           Token::TokenType::TOKEN_PLUS_PLUS,
	           Token::TokenType::TOKEN_MINUS_MINUS})) {
		auto op = previous();
		auto right = unaryExpression();
		return std::make_unique<ast::Unary>(ast::Unary{
		    op,
		    true,
		    std::move(right),
		    op,
		});
	}
	auto expr = call();
	if (match({Token::TokenType::TOKEN_AS})) {
		auto type = typeExpression();
		expr = std::make_unique<ast::Cast>(ast::Cast{
		    std::move(expr),
		    std::move(type),
		    type.getToken(),
		});
	}
	return expr;
}
ast::Type Parser::typeExpression() {
	bool is_mutable = match({Token::TokenType::TOKEN_MUT});
	if (match({Token::TokenType::TOKEN_LEFT_SQUARE_BRACE})) {
		auto arrayStartToken = previous();
		auto arrayType = typeExpression();
		consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after type.");
		consume(Token::TokenType::TOKEN_RIGHT_SQUARE_BRACE,
		        "Expect ']' after array type.");
		return ast::Type{
		    Token{Token::TokenType::TOKEN_LEFT_SQUARE_BRACE,
		          std::format("[{}]",
		                      arrayStartToken.lexeme + arrayType.name.lexeme),
		          arrayStartToken.line, arrayStartToken.column},
		    is_mutable,
		    true,
		    std::make_unique<ast::Type>(std::move(arrayType)),
		    arrayStartToken,
		};
	}
	auto typeToken =
	    consume(Token::TokenType::TOKEN_IDENTIFIER, "Expect type signature");
	std::optional<std::unique_ptr<ast::Type>> subType;
	bool isPointer = false;
	while (match({Token::TokenType::TOKEN_STAR})) {
		subType = std::make_unique<ast::Type>(ast::Type{
		    typeToken,
		    is_mutable,
		    isPointer,
		    std::move(subType),
		    typeToken,
		});
		isPointer = true;
		typeToken.lexeme += previous().getGlyph();
	}
	return ast::Type{
	    typeToken, is_mutable, isPointer, std::move(subType), typeToken,
	};
}

std::unique_ptr<ast::Expression>
Parser::finishArrayAccess(std::unique_ptr<ast::Expression> callee) {
	auto index = expression();
	auto paren = consume(Token::TokenType::TOKEN_RIGHT_SQUARE_BRACE,
	                     "Expect ']' after array access");
	auto token = index->getToken();
	return std::make_unique<ast::ArrayAccess>(ast::ArrayAccess{
	    std::move(callee),
	    std::move(index),
	    token,
	});
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
	auto token = callee->getToken();
	if (token.type == Token::TokenType::TOKEN_INTRINSIC) {
		if (dynamic_cast<ast::Intrinsic *>(callee.get())) {
			return std::make_unique<ast::IntrinsicCall>(
			    std::unique_ptr<ast::Intrinsic>(
			        static_cast<ast::Intrinsic *>(callee.release())),
			    paren, std::move(arguments), token);
		} else {
			error(peek(), std::format("<{}> Expect expression.", "BUG"_red));
		}
	}
	return std::make_unique<ast::Call>(ast::Call{
	    std::move(callee),
	    paren,
	    std::move(arguments),
	    token,
	});
}
std::unique_ptr<ast::Expression> Parser::call() {
	auto expr = primaryExpresion();
	while (true) {
		if (match({Token::TokenType::TOKEN_LEFT_PAREN})) {
			expr = finishCall(std::move(expr));
		} else if (match({Token::TokenType::TOKEN_DOT})) {
			Token name = consume(Token::TokenType::TOKEN_IDENTIFIER,
			                     "Expect property name after '.'.");
			expr = std::make_unique<ast::Get>(ast::Get{
			    std::move(expr),
			    name,
			    name,
			});
		} else if (match({Token::TokenType::TOKEN_LEFT_SQUARE_BRACE})) {
			expr = finishArrayAccess(std::move(expr));
		} else if (match({Token::TokenType::TOKEN_PLUS_PLUS,
		                  Token::TokenType::TOKEN_MINUS_MINUS})) {
			expr = std::make_unique<ast::Unary>(ast::Unary{
			    previous(),
			    false,
			    std::move(expr),
			    previous(),
			});
		} else {
			break;
		}
	}

	return expr;
}

std::unique_ptr<ast::Expression> Parser::primaryExpresion() {
	Token kind = peek();
	if (match({Token::TokenType::TOKEN_FALSE})) {
		return std::make_unique<ast::Literal>(ast::Literal{
		    kind,
		    "false",
		    kind,
		});
	}
	if (match({Token::TokenType::TOKEN_TRUE})) {
		return std::make_unique<ast::Literal>(ast::Literal{
		    kind,
		    "true",
		    kind,
		});
	}

	if (match({Token::TokenType::TOKEN_NUMBER, Token::TokenType::TOKEN_STRING,
	           Token::TokenType::TOKEN_CHAR})) {
		return std::make_unique<ast::Literal>(ast::Literal{
		    kind,
		    previous().lexeme,
		    kind,
		});
	}

	if (match({Token::TokenType::TOKEN_IDENTIFIER})) {
		return std::make_unique<ast::Variable>(ast::Variable{
		    previous(),
		    kind,
		});
	}

	if (match({Token::TokenType::TOKEN_INTRINSIC})) {
		Token token = previous();
		return std::make_unique<ast::Intrinsic>(ast::Intrinsic{
		    token,
		    ast::getintrinsicType(token.lexeme),
		    kind,
		});
	}

	if (match({Token::TokenType::TOKEN_LEFT_PAREN})) {
		auto expr = expression();
		consume(Token::TokenType::TOKEN_RIGHT_PAREN,
		        "Expect ')' after expression.");
		return std::make_unique<ast::Grouping>(ast::Grouping{
		    std::move(expr),
		    kind,
		});
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
	errorBag.error(token, "PARSE", message);
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
		case Token::TokenType::TOKEN_STRUCT:
		default: {
		}
		}

		advance();
	}
}

} // namespace ray::compiler

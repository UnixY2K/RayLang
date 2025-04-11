#pragma once
#include <memory>
#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lexer/token.hpp>

#include <cstddef>
#include <exception>
#include <vector>

namespace ray::compiler {

class ParseException : public std::exception {};

class Parser {
	std::vector<Token> tokens;
	size_t current = 0;

  public:
	Parser() = default;
	Parser(std::vector<Token> tokens) : tokens(tokens) {}

	std::vector<std::unique_ptr<ast::Statement>> parse();

  private:
	ast::Expression expression();
	std::unique_ptr<ast::Statement> declaration();
	std::unique_ptr<ast::Statement> statement();
	ast::Statement forStatement();
	ast::Statement ifStatement();
	ast::Statement returnStatement();
	ast::Statement continueStatement();
	ast::Statement breakStatement();
	ast::Statement whileStatement();
	std::unique_ptr<ast::Statement> varDeclaration();
	ast::Statement expressionStatement();
	ast::Function function(std::string kind);
	std::vector<std::unique_ptr<ast::Statement>> block();
	ast::Expression comma();
	ast::Expression assignment();
	ast::Expression orExpression();
	ast::Expression andExpression();
	ast::Expression equalityExpression();
	ast::Expression comparisonExpression();
	ast::Expression terminalExpression();
	ast::Expression factorExpression();
	ast::Expression unaryExpression();
	ast::Expression finishCall(ast::Expression callee);
	ast::Expression call();
	ast::Expression primaryExpresion();

	bool match(std::vector<Token::TokenType> types);
	bool check(Token::TokenType type);

	Token advance();

	bool isAtEnd() const;
	Token peek() const;
	Token previous();
	Token consume(Token::TokenType type, std::string message);

	ParseException error(Token token, std::string message);

	void synchronize();
};
} // namespace ray::compiler

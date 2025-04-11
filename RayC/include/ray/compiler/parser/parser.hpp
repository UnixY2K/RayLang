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
	std::unique_ptr<ast::Expression> expression();
	std::unique_ptr<ast::Statement> declaration();
	std::unique_ptr<ast::Statement> statement();
	std::unique_ptr<ast::Statement> forStatement();
	std::unique_ptr<ast::Statement> ifStatement();
	std::unique_ptr<ast::Statement> returnStatement();
	std::unique_ptr<ast::Statement> continueStatement();
	std::unique_ptr<ast::Statement> breakStatement();
	std::unique_ptr<ast::Statement> whileStatement();
	std::unique_ptr<ast::Statement> varDeclaration();
	std::unique_ptr<ast::Statement> expressionStatement();
	ast::Function function(std::string kind);
	std::vector<std::unique_ptr<ast::Statement>> block();
	std::unique_ptr<ast::Expression> comma();
	std::unique_ptr<ast::Expression> assignment();
	std::unique_ptr<ast::Expression> orExpression();
	std::unique_ptr<ast::Expression> andExpression();
	std::unique_ptr<ast::Expression> equalityExpression();
	std::unique_ptr<ast::Expression> comparisonExpression();
	std::unique_ptr<ast::Expression> terminalExpression();
	std::unique_ptr<ast::Expression> factorExpression();
	std::unique_ptr<ast::Expression> unaryExpression();
	std::unique_ptr<ast::Expression>
	finishCall(std::unique_ptr<ast::Expression> callee);
	std::unique_ptr<ast::Expression> call();
	std::unique_ptr<ast::Expression> primaryExpresion();

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

#pragma once
#include <cstddef>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/message_bag.hpp>

namespace ray::compiler {

class ParseException : public std::exception {};

class Parser {
	MessageBag messageBag;
	std::vector<Token> tokens;
	size_t current = 0;

  public:
	Parser() : Parser("", {}) {};
	Parser(std::string filepath, std::vector<Token> tokens);

	std::vector<std::unique_ptr<ast::Statement>> parse();

	bool failed() const;
	const std::vector<std::string> getErrors() const;

  private:
	std::unique_ptr<ast::Expression> expression();
	std::optional<std::unique_ptr<ast::Statement>> CompilerDirective();
	std::optional<std::unique_ptr<ast::Statement>> declaration();
	std::unique_ptr<ast::Statement> structDeclaration(bool publicVisibility);
	std::unique_ptr<ast::Statement> statement();
	std::unique_ptr<ast::Statement> forStatement();
	std::unique_ptr<ast::Statement> ifStatement();
	std::unique_ptr<ast::Statement> returnStatement();
	std::unique_ptr<ast::Statement> continueStatement();
	std::unique_ptr<ast::Statement> breakStatement();
	std::unique_ptr<ast::Statement> whileStatement();
	ast::VarDecl varDeclaration();
	ast::Member memberDeclaration();
	std::unique_ptr<ast::Statement> expressionStatement();
	ast::Function function(std::string kind, bool publicVisibility);
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
	std::unique_ptr<ast::Expression> arrayTypeExpression();
	std::unique_ptr<ast::Expression> tupleTypeExpression();
	std::unique_ptr<ast::Expression> pointerTypeExpression();
	std::unique_ptr<ast::Expression> namedTypeExpression();
	std::unique_ptr<ast::Expression>
	finishArrayAccess(std::unique_ptr<ast::Expression> callee);
	std::unique_ptr<ast::Expression>
	finishCall(std::unique_ptr<ast::Expression> callee);
	std::unique_ptr<ast::Expression> call();
	std::unique_ptr<ast::Expression> primaryExpresion();

	bool match(std::vector<Token::TokenType> types);
	bool check(Token::TokenType type);

	Token advance();

	bool isAtEnd() const;
	Token peek() const;
	Token peekNext() const;
	Token previous();
	Token consume(Token::TokenType type, std::string message);

	static constexpr std::unique_ptr<ast::Expression>
	makeUnitExpression(size_t line, size_t column) {
		auto tupleNameToken = Token{Token::TokenType::TOKEN_LEFT_SQUARE_BRACE,
		                            "%<tuple>%", line, column};
		return std::make_unique<ast::TupleType>(
		    ast::TupleType{false, {}, tupleNameToken});
	}

	ParseException error(Token token, std::string message);

	void synchronize();
};
} // namespace ray::compiler

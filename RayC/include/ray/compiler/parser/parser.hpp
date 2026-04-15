#pragma once
#include <cstddef>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/syntax/ast/Expression.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>

namespace ray::compiler {

class ParseException : public std::exception {};

class Parser {
	MessageBag messageBag;
	std::vector<Token> tokens;
	size_t current = 0;

  public:
	Parser() : Parser("", {}) {};
	Parser(std::string filepath, std::vector<Token> tokens);

	std::vector<std::unique_ptr<syntax::ast::Statement>> parse();

	bool failed() const;
	const std::vector<std::string> getErrors() const;

  private:
	std::unique_ptr<syntax::ast::Expression> expression();
	std::optional<std::unique_ptr<syntax::ast::Statement>> CompilerDirective();
	std::optional<std::unique_ptr<syntax::ast::Statement>> declaration();
	std::unique_ptr<syntax::ast::Statement>
	structDeclaration(bool publicVisibility);
	std::unique_ptr<syntax::ast::Statement>
	traitDeclaration(bool publicVisibility);
	std::unique_ptr<syntax::ast::Statement> statement();
	std::unique_ptr<syntax::ast::Statement> forStatement();
	std::unique_ptr<syntax::ast::Statement> ifStatement();
	std::unique_ptr<syntax::ast::Statement> returnStatement();
	std::unique_ptr<syntax::ast::Statement> continueStatement();
	std::unique_ptr<syntax::ast::Statement> breakStatement();
	std::unique_ptr<syntax::ast::Statement> whileStatement();
	syntax::ast::VarDecl varDeclaration();
	syntax::ast::Member memberDeclaration();
	syntax::ast::Method methodDeclaration(bool publicVisibility);
	std::unique_ptr<syntax::ast::Statement> expressionStatement();
	syntax::ast::Function function(bool publicVisibility);
	std::vector<std::unique_ptr<syntax::ast::Statement>> block();
	std::unique_ptr<syntax::ast::Expression> comma();
	std::unique_ptr<syntax::ast::Expression> assignment();
	std::unique_ptr<syntax::ast::Expression> orExpression();
	std::unique_ptr<syntax::ast::Expression> andExpression();
	std::unique_ptr<syntax::ast::Expression> equalityExpression();
	std::unique_ptr<syntax::ast::Expression> comparisonExpression();
	std::unique_ptr<syntax::ast::Expression> terminalExpression();
	std::unique_ptr<syntax::ast::Expression> factorExpression();
	std::unique_ptr<syntax::ast::Expression> unaryExpression();
	std::unique_ptr<syntax::ast::Expression> arrayTypeExpression();
	std::unique_ptr<syntax::ast::Expression> tupleTypeExpression();
	std::unique_ptr<syntax::ast::Expression> pointerTypeExpression();
	std::unique_ptr<syntax::ast::Expression> namedTypeExpression();
	std::unique_ptr<syntax::ast::Expression>
	finishArrayAccess(std::unique_ptr<syntax::ast::Expression> callee);
	std::unique_ptr<syntax::ast::Expression>
	finishCall(std::unique_ptr<syntax::ast::Expression> callee);
	std::unique_ptr<syntax::ast::Expression> call();
	std::unique_ptr<syntax::ast::Expression> primaryExpresion();

	bool match(std::vector<Token::TokenType> types);
	bool check(Token::TokenType type);

	Token advance();

	bool isAtEnd() const;
	Token peek() const;
	Token peekNext() const;
	Token previous();
	Token consume(Token::TokenType type, std::string message);

	static constexpr std::unique_ptr<syntax::ast::Expression>
	makeUnitExpression(size_t line, size_t column) {
		auto tupleNameToken = Token{Token::TokenType::TOKEN_LEFT_SQUARE_BRACE,
		                            "%<tuple>%", line, column};
		return std::make_unique<syntax::ast::TupleType>(
		    syntax::ast::TupleType{false, {}, tupleNameToken});
	}

	ParseException error(Token token, std::string message);

	void synchronize();
};
} // namespace ray::compiler

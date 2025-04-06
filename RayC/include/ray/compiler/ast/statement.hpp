#pragma once
#include <memory>
#include <vector>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/expression.hpp>

namespace ray::compiler::ast {

class Statement {
  public:
};

class Block : public Statement {
  public:
	std::unique_ptr<std::vector<Statement>> statements;

	Block(std::unique_ptr<std::vector<Statement>> statements)
	    : statements(std::move(statements)) {}

};
class TerminalExpr : public Statement {
  public:
	std::unique_ptr<Expression> expression;

	TerminalExpr(std::unique_ptr<Expression> expression)
	    : expression(std::move(expression)) {}

};
class ExpressionStmt : public Statement {
  public:
	std::unique_ptr<Expression> expression;

	ExpressionStmt(std::unique_ptr<Expression> expression)
	    : expression(std::move(expression)) {}

};
class Function : public Statement {
  public:
	std::unique_ptr<Token> name;
	std::unique_ptr<std::vector<Token>> params;
	std::unique_ptr<std::vector<std::unique_ptr<Statement>>> body;

	Function(std::unique_ptr<Token> name,
	        std::unique_ptr<std::vector<Token>> params,
	        std::unique_ptr<std::vector<std::unique_ptr<Statement>>> body)
	    : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

};
class If : public Statement {
  public:
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement> thenBranch;
	std::unique_ptr<Statement> elseBranch;

	If(std::unique_ptr<Expression> condition,
	        std::unique_ptr<Statement> thenBranch,
	        std::unique_ptr<Statement> elseBranch)
	    : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

};
class Print : public Statement {
  public:
	std::unique_ptr<Expression> expression;

	Print(std::unique_ptr<Expression> expression)
	    : expression(std::move(expression)) {}

};
class Jump : public Statement {
  public:
	std::unique_ptr<Token> keyword;
	std::unique_ptr<Expression> value;

	Jump(std::unique_ptr<Token> keyword,
	        std::unique_ptr<Expression> value)
	    : keyword(std::move(keyword)), value(std::move(value)) {}

};
class Var : public Statement {
  public:
	std::unique_ptr<Token> name;
	std::unique_ptr<Expression> initializer;

	Var(std::unique_ptr<Token> name,
	        std::unique_ptr<Expression> initializer)
	    : name(std::move(name)), initializer(std::move(initializer)) {}

};
class While : public Statement {
  public:
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement> body;

	While(std::unique_ptr<Expression> condition,
	        std::unique_ptr<Statement> body)
	    : condition(std::move(condition)), body(std::move(body)) {}

};

} // namespace ray::compiler::ast

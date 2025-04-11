#pragma once
#include <memory>
#include <vector>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/expression.hpp>

namespace ray::compiler::ast {

class Statement {
  public:
	virtual ~Statement() = default;
};

class Block : public Statement {
  public:
	std::vector<std::unique_ptr<Statement>> statements;

	Block(std::vector<std::unique_ptr<Statement>> statements)
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
	Token name;
	std::vector<Token> params;
	std::vector<std::unique_ptr<Statement>> body;

	Function(Token name,
	        std::vector<Token> params,
	        std::vector<std::unique_ptr<Statement>> body)
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
class Jump : public Statement {
  public:
	Token keyword;
	std::unique_ptr<Expression> value;

	Jump(Token keyword,
	        std::unique_ptr<Expression> value)
	    : keyword(std::move(keyword)), value(std::move(value)) {}

};
class Var : public Statement {
  public:
	Token name;
	std::unique_ptr<Expression> initializer;

	Var(Token name,
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

#pragma once
#include <memory>
#include <vector>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/expression.hpp>

namespace ray::compiler::ast {

template <class T> class StatementVisitor;

template <class T> class Statement {
  public:
	virtual T accept(StatementVisitor<T> &visitor) = 0;
};

template <class T> class Block : Statement<T> {
  public:
	std::unique_ptr<std::vector<Statement<T>>> statements;

	Block(std::unique_ptr<std::vector<Statement<T>>> statements)
	    : statements(std::move(statements)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitBlockStatement(this);
	}
};

template <class T> class TerminalExpr : Statement<T> {
  public:
	std::unique_ptr<Expression<T>> expression;

	TerminalExpr(std::unique_ptr<Expression<T>> expression)
	    : expression(std::move(expression)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitTerminalExprStatement(this);
	}
};

template <class T> class ExpressionStmt : Statement<T> {
  public:
	std::unique_ptr<Expression<T>> expression;

	ExpressionStmt(std::unique_ptr<Expression<T>> expression)
	    : expression(std::move(expression)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitExpressionStmtStatement(this);
	}
};

template <class T> class Function : Statement<T> {
  public:
	std::unique_ptr<Token> name;
	std::unique_ptr<std::vector<Token>> params;
	std::unique_ptr<std::vector<Statement<T>>> body;

	Function(std::unique_ptr<Token> name,
	        std::unique_ptr<std::vector<Token>> params,
	        std::unique_ptr<std::vector<Statement<T>>> body)
	    : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitFunctionStatement(this);
	}
};

template <class T> class If : Statement<T> {
  public:
	std::unique_ptr<Expression<T>> condition;
	std::unique_ptr<Statement<T>> thenBranch;
	std::unique_ptr<Statement<T>> elseBranch;

	If(std::unique_ptr<Expression<T>> condition,
	        std::unique_ptr<Statement<T>> thenBranch,
	        std::unique_ptr<Statement<T>> elseBranch)
	    : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitIfStatement(this);
	}
};

template <class T> class Print : Statement<T> {
  public:
	std::unique_ptr<Expression<T>> expression;

	Print(std::unique_ptr<Expression<T>> expression)
	    : expression(std::move(expression)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitPrintStatement(this);
	}
};

template <class T> class Jump : Statement<T> {
  public:
	std::unique_ptr<Token> keyword;
	std::unique_ptr<Expression<T>> value;

	Jump(std::unique_ptr<Token> keyword,
	        std::unique_ptr<Expression<T>> value)
	    : keyword(std::move(keyword)), value(std::move(value)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitJumpStatement(this);
	}
};

template <class T> class Var : Statement<T> {
  public:
	std::unique_ptr<Token> name;
	std::unique_ptr<Expression<T>> initializer;

	Var(std::unique_ptr<Token> name,
	        std::unique_ptr<Expression<T>> initializer)
	    : name(std::move(name)), initializer(std::move(initializer)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitVarStatement(this);
	}
};

template <class T> class While : Statement<T> {
  public:
	std::unique_ptr<Expression<T>> condition;
	std::unique_ptr<Statement<T>> body;

	While(std::unique_ptr<Expression<T>> condition,
	        std::unique_ptr<Statement<T>> body)
	    : condition(std::move(condition)), body(std::move(body)) {}

	T accept(StatementVisitor<T> &visitor) override {
		return visitor.visitWhileStatement(this);
	}
};


template <class T> class StatementVisitor {
	virtual T visitBlockExpression(Block<T> &ternary) = 0;
	virtual T visitTerminalExprExpression(TerminalExpr<T> &ternary) = 0;
	virtual T visitExpressionStmtExpression(ExpressionStmt<T> &ternary) = 0;
	virtual T visitFunctionExpression(Function<T> &ternary) = 0;
	virtual T visitIfExpression(If<T> &ternary) = 0;
	virtual T visitPrintExpression(Print<T> &ternary) = 0;
	virtual T visitJumpExpression(Jump<T> &ternary) = 0;
	virtual T visitVarExpression(Var<T> &ternary) = 0;
	virtual T visitWhileExpression(While<T> &ternary) = 0;
};

} // namespace ray::compiler::ast

#pragma once
#include <memory>
#include <vector>
#include <optional>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/expression.hpp>

namespace ray::compiler::ast {

class Block;
class TerminalExpr;
class ExpressionStmt;
class Function;
class If;
class Jump;
class Var;
class While;

class StatementVisitor {
  public:
	virtual void visitBlockStatement(const Block& value) = 0;
	virtual void visitTerminalExprStatement(const TerminalExpr& value) = 0;
	virtual void visitExpressionStmtStatement(const ExpressionStmt& value) = 0;
	virtual void visitFunctionStatement(const Function& value) = 0;
	virtual void visitIfStatement(const If& value) = 0;
	virtual void visitJumpStatement(const Jump& value) = 0;
	virtual void visitVarStatement(const Var& value) = 0;
	virtual void visitWhileStatement(const While& value) = 0;
	virtual ~StatementVisitor() = default;
};

class Statement {
  public:
	virtual void visit(StatementVisitor& visitor) const = 0;
	virtual ~Statement() = default;
};

class Block : public Statement {
  public:
	std::vector<std::unique_ptr<Statement>> statements;

	Block(std::vector<std::unique_ptr<Statement>> statements)
	    : statements(std::move(statements)) {}

	void visit(StatementVisitor& visitor) const override { visitor.visitBlockStatement(*this); }

};
class TerminalExpr : public Statement {
  public:
	std::optional<std::unique_ptr<Expression>> expression;

	TerminalExpr(std::optional<std::unique_ptr<Expression>> expression)
	    : expression(std::move(expression)) {}

	void visit(StatementVisitor& visitor) const override { visitor.visitTerminalExprStatement(*this); }

};
class ExpressionStmt : public Statement {
  public:
	std::unique_ptr<Expression> expression;

	ExpressionStmt(std::unique_ptr<Expression> expression)
	    : expression(std::move(expression)) {}

	void visit(StatementVisitor& visitor) const override { visitor.visitExpressionStmtStatement(*this); }

};
class Function : public Statement {
  public:
	bool publicVisibility;
	Token name;
	std::vector<Parameter> params;
	Block body;
	Type returnType;

	Function(bool publicVisibility,
	        Token name,
	        std::vector<Parameter> params,
	        Block body,
	        Type returnType)
	    : publicVisibility(std::move(publicVisibility)), name(std::move(name)), params(std::move(params)), body(std::move(body)), returnType(std::move(returnType)) {}

	void visit(StatementVisitor& visitor) const override { visitor.visitFunctionStatement(*this); }

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

	void visit(StatementVisitor& visitor) const override { visitor.visitIfStatement(*this); }

};
class Jump : public Statement {
  public:
	Token keyword;
	std::unique_ptr<Expression> value;

	Jump(Token keyword,
	        std::unique_ptr<Expression> value)
	    : keyword(std::move(keyword)), value(std::move(value)) {}

	void visit(StatementVisitor& visitor) const override { visitor.visitJumpStatement(*this); }

};
class Var : public Statement {
  public:
	Token name;
	std::unique_ptr<Expression> initializer;

	Var(Token name,
	        std::unique_ptr<Expression> initializer)
	    : name(std::move(name)), initializer(std::move(initializer)) {}

	void visit(StatementVisitor& visitor) const override { visitor.visitVarStatement(*this); }

};
class While : public Statement {
  public:
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement> body;

	While(std::unique_ptr<Expression> condition,
	        std::unique_ptr<Statement> body)
	    : condition(std::move(condition)), body(std::move(body)) {}

	void visit(StatementVisitor& visitor) const override { visitor.visitWhileStatement(*this); }

};

} // namespace ray::compiler::ast

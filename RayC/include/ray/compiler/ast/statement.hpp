#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/expression.hpp>

namespace ray::compiler::ast {

using CompDirectiveAttr = std::unordered_map<std::string, std::string>;

class Block;
class TerminalExpr;
class ExpressionStmt;
class Function;
class If;
class Jump;
class Var;
class While;
class Struct;
class CompDirective;

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
	virtual void visitStructStatement(const Struct& value) = 0;
	virtual void visitCompDirectiveStatement(const CompDirective& value) = 0;
	virtual ~StatementVisitor() = default;
};

class Statement {
  public:
	virtual void visit(StatementVisitor& visitor) const = 0;
	virtual const std::string_view variantName() const = 0;
	virtual const Token& getToken() const = 0;
	virtual ~Statement() = default;
};

class Block : public Statement {
  public:
	std::vector<std::unique_ptr<Statement>> statements;
	Token token;

	Block(std::vector<std::unique_ptr<Statement>> statements,
	        Token token):
		statements(std::move(statements)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitBlockStatement(*this);
	}

	const std::string_view variantName() const override { return "Block"; }

	const Token& getToken() const override { return token; };
};
class TerminalExpr : public Statement {
  public:
	std::optional<std::unique_ptr<Expression>> expression;
	Token token;

	TerminalExpr(std::optional<std::unique_ptr<Expression>> expression,
	        Token token):
		expression(std::move(expression)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitTerminalExprStatement(*this);
	}

	const std::string_view variantName() const override { return "TerminalExpr"; }

	const Token& getToken() const override { return token; };
};
class ExpressionStmt : public Statement {
  public:
	std::unique_ptr<Expression> expression;
	Token token;

	ExpressionStmt(std::unique_ptr<Expression> expression,
	        Token token):
		expression(std::move(expression)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitExpressionStmtStatement(*this);
	}

	const std::string_view variantName() const override { return "ExpressionStmt"; }

	const Token& getToken() const override { return token; };
};
class Function : public Statement {
  public:
	Token name;
	bool publicVisibility;
	std::vector<Parameter> params;
	std::optional<Block> body;
	Type returnType;
	Token token;

	Function(Token name,
	        bool publicVisibility,
	        std::vector<Parameter> params,
	        std::optional<Block> body,
	        Type returnType,
	        Token token):
		name(std::move(name)),
		publicVisibility(std::move(publicVisibility)),
		params(std::move(params)),
		body(std::move(body)),
		returnType(std::move(returnType)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitFunctionStatement(*this);
	}

	const std::string_view variantName() const override { return "Function"; }

	const Token& getToken() const override { return token; };
};
class If : public Statement {
  public:
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement> thenBranch;
	std::optional<std::unique_ptr<Statement>> elseBranch;
	Token token;

	If(std::unique_ptr<Expression> condition,
	        std::unique_ptr<Statement> thenBranch,
	        std::optional<std::unique_ptr<Statement>> elseBranch,
	        Token token):
		condition(std::move(condition)),
		thenBranch(std::move(thenBranch)),
		elseBranch(std::move(elseBranch)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitIfStatement(*this);
	}

	const std::string_view variantName() const override { return "If"; }

	const Token& getToken() const override { return token; };
};
class Jump : public Statement {
  public:
	Token keyword;
	std::optional<std::unique_ptr<Expression>> value;
	Token token;

	Jump(Token keyword,
	        std::optional<std::unique_ptr<Expression>> value,
	        Token token):
		keyword(std::move(keyword)),
		value(std::move(value)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitJumpStatement(*this);
	}

	const std::string_view variantName() const override { return "Jump"; }

	const Token& getToken() const override { return token; };
};
class Var : public Statement {
  public:
	Token name;
	Type type;
	bool is_mutable;
	std::optional<std::unique_ptr<Expression>> initializer;
	Token token;

	Var(Token name,
	        Type type,
	        bool is_mutable,
	        std::optional<std::unique_ptr<Expression>> initializer,
	        Token token):
		name(std::move(name)),
		type(std::move(type)),
		is_mutable(std::move(is_mutable)),
		initializer(std::move(initializer)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitVarStatement(*this);
	}

	const std::string_view variantName() const override { return "Var"; }

	const Token& getToken() const override { return token; };
};
class While : public Statement {
  public:
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement> body;
	Token token;

	While(std::unique_ptr<Expression> condition,
	        std::unique_ptr<Statement> body,
	        Token token):
		condition(std::move(condition)),
		body(std::move(body)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitWhileStatement(*this);
	}

	const std::string_view variantName() const override { return "While"; }

	const Token& getToken() const override { return token; };
};
class Struct : public Statement {
  public:
	Token name;
	bool publicVisibility;
	bool declaration;
	std::vector<Var> members;
	std::vector<bool> memberVisibility;
	Token token;

	Struct(Token name,
	        bool publicVisibility,
	        bool declaration,
	        std::vector<Var> members,
	        std::vector<bool> memberVisibility,
	        Token token):
		name(std::move(name)),
		publicVisibility(std::move(publicVisibility)),
		declaration(std::move(declaration)),
		members(std::move(members)),
		memberVisibility(std::move(memberVisibility)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitStructStatement(*this);
	}

	const std::string_view variantName() const override { return "Struct"; }

	const Token& getToken() const override { return token; };
};
class CompDirective : public Statement {
  public:
	Token name;
	CompDirectiveAttr values;
	std::unique_ptr<Statement> child;
	Token token;

	CompDirective(Token name,
	        CompDirectiveAttr values,
	        std::unique_ptr<Statement> child,
	        Token token):
		name(std::move(name)),
		values(std::move(values)),
		child(std::move(child)),
		token(std::move(token)) {}

	void visit(StatementVisitor& visitor) const override {
		visitor.visitCompDirectiveStatement(*this);
	}

	const std::string_view variantName() const override { return "CompDirective"; }

	const Token& getToken() const override { return token; };
};

} // namespace ray::compiler::ast

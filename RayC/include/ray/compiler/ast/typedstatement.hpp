#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/typed/typedExpression.hpp>

namespace ray::compiler::ast {

using CompDirectiveAttr = std::unordered_map<std::string, std::string>;

class Block;
class TerminalExpr;
class ExpressionStmt;
class Function;
class Method;
class If;
class Jump;
class VarDecl;
class Member;
class While;
class Struct;
class Trait;
class CompDirective;

class TypedStatementVisitor {
  public:
	virtual void visitBlockTypedStatement(const Block& value) = 0;
	virtual void visitTerminalExprTypedStatement(const TerminalExpr& value) = 0;
	virtual void visitExpressionStmtTypedStatement(const ExpressionStmt& value) = 0;
	virtual void visitFunctionTypedStatement(const Function& value) = 0;
	virtual void visitMethodTypedStatement(const Method& value) = 0;
	virtual void visitIfTypedStatement(const If& value) = 0;
	virtual void visitJumpTypedStatement(const Jump& value) = 0;
	virtual void visitVarDeclTypedStatement(const VarDecl& value) = 0;
	virtual void visitMemberTypedStatement(const Member& value) = 0;
	virtual void visitWhileTypedStatement(const While& value) = 0;
	virtual void visitStructTypedStatement(const Struct& value) = 0;
	virtual void visitTraitTypedStatement(const Trait& value) = 0;
	virtual void visitCompDirectiveTypedStatement(const CompDirective& value) = 0;
	virtual ~TypedStatementVisitor() = default;
};

class TypedStatement {
  public:
	virtual void visit(TypedStatementVisitor& visitor) const = 0;
	virtual const std::string_view variantName() const = 0;
	virtual const Token& getToken() const = 0;
	virtual ~TypedStatement() = default;
};

class Block : public TypedStatement {
  public:
	std::vector<std::unique_ptr<TypedStatement>> statements;
	Token token;

	Block(std::vector<std::unique_ptr<TypedStatement>> statements,
	        Token token):
		statements(std::move(statements)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitBlockTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "Block"; }

	const Token& getToken() const override { return token; };
};
class TerminalExpr : public TypedStatement {
  public:
	std::optional<std::unique_ptr<TypedExpression>> expression;
	Token token;

	TerminalExpr(std::optional<std::unique_ptr<TypedExpression>> expression,
	        Token token):
		expression(std::move(expression)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitTerminalExprTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "TerminalExpr"; }

	const Token& getToken() const override { return token; };
};
class ExpressionStmt : public TypedStatement {
  public:
	std::unique_ptr<TypedExpression> expression;
	Token token;

	ExpressionStmt(std::unique_ptr<TypedExpression> expression,
	        Token token):
		expression(std::move(expression)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitExpressionStmtTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "ExpressionStmt"; }

	const Token& getToken() const override { return token; };
};
class Function : public TypedStatement {
  public:
	Token name;
	bool publicVisibility;
	std::vector<Parameter> params;
	std::optional<Block> body;
	std::unique_ptr<ast::TypedExpression> returnType;
	Token token;

	Function(Token name,
	        bool publicVisibility,
	        std::vector<Parameter> params,
	        std::optional<Block> body,
	        std::unique_ptr<ast::TypedExpression> returnType,
	        Token token):
		name(std::move(name)),
		publicVisibility(std::move(publicVisibility)),
		params(std::move(params)),
		body(std::move(body)),
		returnType(std::move(returnType)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitFunctionTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "Function"; }

	const Token& getToken() const override { return token; };
};
class Method : public TypedStatement {
  public:
	Token name;
	bool publicVisibility;
	std::vector<Parameter> params;
	std::optional<Block> body;
	std::unique_ptr<ast::TypedExpression> returnType;
	Token token;

	Method(Token name,
	        bool publicVisibility,
	        std::vector<Parameter> params,
	        std::optional<Block> body,
	        std::unique_ptr<ast::TypedExpression> returnType,
	        Token token):
		name(std::move(name)),
		publicVisibility(std::move(publicVisibility)),
		params(std::move(params)),
		body(std::move(body)),
		returnType(std::move(returnType)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitMethodTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "Method"; }

	const Token& getToken() const override { return token; };
};
class If : public TypedStatement {
  public:
	std::unique_ptr<TypedExpression> condition;
	std::unique_ptr<TypedStatement> thenBranch;
	std::optional<std::unique_ptr<TypedStatement>> elseBranch;
	Token token;

	If(std::unique_ptr<TypedExpression> condition,
	        std::unique_ptr<TypedStatement> thenBranch,
	        std::optional<std::unique_ptr<TypedStatement>> elseBranch,
	        Token token):
		condition(std::move(condition)),
		thenBranch(std::move(thenBranch)),
		elseBranch(std::move(elseBranch)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitIfTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "If"; }

	const Token& getToken() const override { return token; };
};
class Jump : public TypedStatement {
  public:
	Token keyword;
	std::optional<std::unique_ptr<TypedExpression>> returnValue;
	Token token;

	Jump(Token keyword,
	        std::optional<std::unique_ptr<TypedExpression>> returnValue,
	        Token token):
		keyword(std::move(keyword)),
		returnValue(std::move(returnValue)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitJumpTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "Jump"; }

	const Token& getToken() const override { return token; };
};
class VarDecl : public TypedStatement {
  public:
	Token name;
	std::unique_ptr<TypedExpression> type;
	bool is_mutable;
	std::optional<std::unique_ptr<TypedExpression>> initializer;
	Token token;

	VarDecl(Token name,
	        std::unique_ptr<TypedExpression> type,
	        bool is_mutable,
	        std::optional<std::unique_ptr<TypedExpression>> initializer,
	        Token token):
		name(std::move(name)),
		type(std::move(type)),
		is_mutable(std::move(is_mutable)),
		initializer(std::move(initializer)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitVarDeclTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "VarDecl"; }

	const Token& getToken() const override { return token; };
};
class Member : public TypedStatement {
  public:
	Token name;
	std::unique_ptr<TypedExpression> type;
	bool is_mutable;
	std::optional<std::unique_ptr<TypedExpression>> initializer;
	Token token;

	Member(Token name,
	        std::unique_ptr<TypedExpression> type,
	        bool is_mutable,
	        std::optional<std::unique_ptr<TypedExpression>> initializer,
	        Token token):
		name(std::move(name)),
		type(std::move(type)),
		is_mutable(std::move(is_mutable)),
		initializer(std::move(initializer)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitMemberTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "Member"; }

	const Token& getToken() const override { return token; };
};
class While : public TypedStatement {
  public:
	std::unique_ptr<TypedExpression> condition;
	std::unique_ptr<TypedStatement> body;
	Token token;

	While(std::unique_ptr<TypedExpression> condition,
	        std::unique_ptr<TypedStatement> body,
	        Token token):
		condition(std::move(condition)),
		body(std::move(body)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitWhileTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "While"; }

	const Token& getToken() const override { return token; };
};
class Struct : public TypedStatement {
  public:
	Token name;
	bool publicVisibility;
	bool declaration;
	std::vector<Member> members;
	std::vector<bool> memberVisibility;
	Token token;

	Struct(Token name,
	        bool publicVisibility,
	        bool declaration,
	        std::vector<Member> members,
	        std::vector<bool> memberVisibility,
	        Token token):
		name(std::move(name)),
		publicVisibility(std::move(publicVisibility)),
		declaration(std::move(declaration)),
		members(std::move(members)),
		memberVisibility(std::move(memberVisibility)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitStructTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "Struct"; }

	const Token& getToken() const override { return token; };
};
class Trait : public TypedStatement {
  public:
	Token name;
	bool publicVisibility;
	std::vector<Method> methods;
	Token token;

	Trait(Token name,
	        bool publicVisibility,
	        std::vector<Method> methods,
	        Token token):
		name(std::move(name)),
		publicVisibility(std::move(publicVisibility)),
		methods(std::move(methods)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitTraitTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "Trait"; }

	const Token& getToken() const override { return token; };
};
class CompDirective : public TypedStatement {
  public:
	Token name;
	CompDirectiveAttr values;
	std::unique_ptr<TypedStatement> child;
	Token token;

	CompDirective(Token name,
	        CompDirectiveAttr values,
	        std::unique_ptr<TypedStatement> child,
	        Token token):
		name(std::move(name)),
		values(std::move(values)),
		child(std::move(child)),
		token(std::move(token)) {}

	void visit(TypedStatementVisitor& visitor) const override {
		visitor.visitCompDirectiveTypedStatement(*this);
	}

	const std::string_view variantName() const override { return "CompDirective"; }

	const Token& getToken() const override { return token; };
};

} // namespace ray::compiler::ast

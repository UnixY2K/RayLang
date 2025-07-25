#pragma once
#include <memory>
#include <vector>
#include <optional>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/intrinsic.hpp>

namespace ray::compiler::ast {

class Variable;
class Intrinsic;
class Assign;
class Binary;
class Call;
class IntrinsicCall;
class Get;
class Grouping;
class Literal;
class Logical;
class Set;
class Unary;
class ArrayAccess;
class Type;
class Cast;
class Parameter;

class ExpressionVisitor {
  public:
	virtual void visitVariableExpression(const Variable& value) = 0;
	virtual void visitIntrinsicExpression(const Intrinsic& value) = 0;
	virtual void visitAssignExpression(const Assign& value) = 0;
	virtual void visitBinaryExpression(const Binary& value) = 0;
	virtual void visitCallExpression(const Call& value) = 0;
	virtual void visitIntrinsicCallExpression(const IntrinsicCall& value) = 0;
	virtual void visitGetExpression(const Get& value) = 0;
	virtual void visitGroupingExpression(const Grouping& value) = 0;
	virtual void visitLiteralExpression(const Literal& value) = 0;
	virtual void visitLogicalExpression(const Logical& value) = 0;
	virtual void visitSetExpression(const Set& value) = 0;
	virtual void visitUnaryExpression(const Unary& value) = 0;
	virtual void visitArrayAccessExpression(const ArrayAccess& value) = 0;
	virtual void visitTypeExpression(const Type& value) = 0;
	virtual void visitCastExpression(const Cast& value) = 0;
	virtual void visitParameterExpression(const Parameter& value) = 0;
	virtual ~ExpressionVisitor() = default;
};

class Expression {
  public:
	virtual void visit(ExpressionVisitor& visitor) const = 0;
	virtual const std::string_view variantName() const = 0;
	virtual const Token& getToken() const = 0;
	virtual ~Expression() = default;
};

class Variable : public Expression {
  public:
	Token name;
	Token token;

	Variable(Token name,
	        Token token):
		name(std::move(name)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitVariableExpression(*this);
	}

	const std::string_view variantName() const override { return "Variable"; }

	const Token& getToken() const override { return token; };
};
class Intrinsic : public Expression {
  public:
	Token name;
	IntrinsicType intrinsic;
	Token token;

	Intrinsic(Token name,
	        IntrinsicType intrinsic,
	        Token token):
		name(std::move(name)),
		intrinsic(std::move(intrinsic)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitIntrinsicExpression(*this);
	}

	const std::string_view variantName() const override { return "Intrinsic"; }

	const Token& getToken() const override { return token; };
};
class Assign : public Expression {
  public:
	std::unique_ptr<Expression> lhs;
	Token assignmentOp;
	std::unique_ptr<Expression> rhs;
	Token token;

	Assign(std::unique_ptr<Expression> lhs,
	        Token assignmentOp,
	        std::unique_ptr<Expression> rhs,
	        Token token):
		lhs(std::move(lhs)),
		assignmentOp(std::move(assignmentOp)),
		rhs(std::move(rhs)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitAssignExpression(*this);
	}

	const std::string_view variantName() const override { return "Assign"; }

	const Token& getToken() const override { return token; };
};
class Binary : public Expression {
  public:
	std::unique_ptr<Expression> left;
	Token op;
	std::unique_ptr<Expression> right;
	Token token;

	Binary(std::unique_ptr<Expression> left,
	        Token op,
	        std::unique_ptr<Expression> right,
	        Token token):
		left(std::move(left)),
		op(std::move(op)),
		right(std::move(right)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitBinaryExpression(*this);
	}

	const std::string_view variantName() const override { return "Binary"; }

	const Token& getToken() const override { return token; };
};
class Call : public Expression {
  public:
	std::unique_ptr<Expression> callee;
	Token paren;
	std::vector<std::unique_ptr<Expression>> arguments;
	Token token;

	Call(std::unique_ptr<Expression> callee,
	        Token paren,
	        std::vector<std::unique_ptr<Expression>> arguments,
	        Token token):
		callee(std::move(callee)),
		paren(std::move(paren)),
		arguments(std::move(arguments)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitCallExpression(*this);
	}

	const std::string_view variantName() const override { return "Call"; }

	const Token& getToken() const override { return token; };
};
class IntrinsicCall : public Expression {
  public:
	std::unique_ptr<Intrinsic> callee;
	Token paren;
	std::vector<std::unique_ptr<Expression>> arguments;
	Token token;

	IntrinsicCall(std::unique_ptr<Intrinsic> callee,
	        Token paren,
	        std::vector<std::unique_ptr<Expression>> arguments,
	        Token token):
		callee(std::move(callee)),
		paren(std::move(paren)),
		arguments(std::move(arguments)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitIntrinsicCallExpression(*this);
	}

	const std::string_view variantName() const override { return "IntrinsicCall"; }

	const Token& getToken() const override { return token; };
};
class Get : public Expression {
  public:
	std::unique_ptr<Expression> object;
	Token name;
	Token token;

	Get(std::unique_ptr<Expression> object,
	        Token name,
	        Token token):
		object(std::move(object)),
		name(std::move(name)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitGetExpression(*this);
	}

	const std::string_view variantName() const override { return "Get"; }

	const Token& getToken() const override { return token; };
};
class Grouping : public Expression {
  public:
	std::unique_ptr<Expression> expression;
	Token token;

	Grouping(std::unique_ptr<Expression> expression,
	        Token token):
		expression(std::move(expression)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitGroupingExpression(*this);
	}

	const std::string_view variantName() const override { return "Grouping"; }

	const Token& getToken() const override { return token; };
};
class Literal : public Expression {
  public:
	Token kind;
	std::string value;
	Token token;

	Literal(Token kind,
	        std::string value,
	        Token token):
		kind(std::move(kind)),
		value(std::move(value)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitLiteralExpression(*this);
	}

	const std::string_view variantName() const override { return "Literal"; }

	const Token& getToken() const override { return token; };
};
class Logical : public Expression {
  public:
	std::unique_ptr<Expression> left;
	Token op;
	std::unique_ptr<Expression> right;
	Token token;

	Logical(std::unique_ptr<Expression> left,
	        Token op,
	        std::unique_ptr<Expression> right,
	        Token token):
		left(std::move(left)),
		op(std::move(op)),
		right(std::move(right)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitLogicalExpression(*this);
	}

	const std::string_view variantName() const override { return "Logical"; }

	const Token& getToken() const override { return token; };
};
class Set : public Expression {
  public:
	std::unique_ptr<Expression> object;
	Token name;
	Token assignmentOp;
	std::unique_ptr<Expression> value;
	Token token;

	Set(std::unique_ptr<Expression> object,
	        Token name,
	        Token assignmentOp,
	        std::unique_ptr<Expression> value,
	        Token token):
		object(std::move(object)),
		name(std::move(name)),
		assignmentOp(std::move(assignmentOp)),
		value(std::move(value)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitSetExpression(*this);
	}

	const std::string_view variantName() const override { return "Set"; }

	const Token& getToken() const override { return token; };
};
class Unary : public Expression {
  public:
	Token op;
	bool isPrefix;
	std::unique_ptr<Expression> expr;
	Token token;

	Unary(Token op,
	        bool isPrefix,
	        std::unique_ptr<Expression> expr,
	        Token token):
		op(std::move(op)),
		isPrefix(std::move(isPrefix)),
		expr(std::move(expr)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitUnaryExpression(*this);
	}

	const std::string_view variantName() const override { return "Unary"; }

	const Token& getToken() const override { return token; };
};
class ArrayAccess : public Expression {
  public:
	std::unique_ptr<Expression> array;
	std::unique_ptr<Expression> index;
	Token token;

	ArrayAccess(std::unique_ptr<Expression> array,
	        std::unique_ptr<Expression> index,
	        Token token):
		array(std::move(array)),
		index(std::move(index)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitArrayAccessExpression(*this);
	}

	const std::string_view variantName() const override { return "ArrayAccess"; }

	const Token& getToken() const override { return token; };
};
class Type : public Expression {
  public:
	Token name;
	bool isMutable;
	bool isPointer;
	std::optional<std::unique_ptr<Type>> subtype;
	Token token;

	Type(Token name,
	        bool isMutable,
	        bool isPointer,
	        std::optional<std::unique_ptr<Type>> subtype,
	        Token token):
		name(std::move(name)),
		isMutable(std::move(isMutable)),
		isPointer(std::move(isPointer)),
		subtype(std::move(subtype)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitTypeExpression(*this);
	}

	const std::string_view variantName() const override { return "Type"; }

	const Token& getToken() const override { return token; };
};
class Cast : public Expression {
  public:
	std::unique_ptr<Expression> expression;
	Type type;
	Token token;

	Cast(std::unique_ptr<Expression> expression,
	        Type type,
	        Token token):
		expression(std::move(expression)),
		type(std::move(type)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitCastExpression(*this);
	}

	const std::string_view variantName() const override { return "Cast"; }

	const Token& getToken() const override { return token; };
};
class Parameter : public Expression {
  public:
	Token name;
	Type type;
	Token token;

	Parameter(Token name,
	        Type type,
	        Token token):
		name(std::move(name)),
		type(std::move(type)),
		token(std::move(token)) {}

	void visit(ExpressionVisitor& visitor) const override {
		visitor.visitParameterExpression(*this);
	}

	const std::string_view variantName() const override { return "Parameter"; }

	const Token& getToken() const override { return token; };
};

} // namespace ray::compiler::ast

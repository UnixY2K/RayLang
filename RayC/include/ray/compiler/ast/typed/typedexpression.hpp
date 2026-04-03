#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/intrinsic.hpp>

namespace ray::compiler::ast {

using CompDirectiveAttr = std::unordered_map<std::string, std::string>;

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
class ArrayType;
class TupleType;
class PointerType;
class NamedType;
class Cast;
class Parameter;

class TypedExpressionVisitor {
  public:
	virtual void visitVariableTypedExpression(const Variable& value) = 0;
	virtual void visitIntrinsicTypedExpression(const Intrinsic& value) = 0;
	virtual void visitAssignTypedExpression(const Assign& value) = 0;
	virtual void visitBinaryTypedExpression(const Binary& value) = 0;
	virtual void visitCallTypedExpression(const Call& value) = 0;
	virtual void visitIntrinsicCallTypedExpression(const IntrinsicCall& value) = 0;
	virtual void visitGetTypedExpression(const Get& value) = 0;
	virtual void visitGroupingTypedExpression(const Grouping& value) = 0;
	virtual void visitLiteralTypedExpression(const Literal& value) = 0;
	virtual void visitLogicalTypedExpression(const Logical& value) = 0;
	virtual void visitSetTypedExpression(const Set& value) = 0;
	virtual void visitUnaryTypedExpression(const Unary& value) = 0;
	virtual void visitArrayAccessTypedExpression(const ArrayAccess& value) = 0;
	virtual void visitArrayTypeTypedExpression(const ArrayType& value) = 0;
	virtual void visitTupleTypeTypedExpression(const TupleType& value) = 0;
	virtual void visitPointerTypeTypedExpression(const PointerType& value) = 0;
	virtual void visitNamedTypeTypedExpression(const NamedType& value) = 0;
	virtual void visitCastTypedExpression(const Cast& value) = 0;
	virtual void visitParameterTypedExpression(const Parameter& value) = 0;
	virtual ~TypedExpressionVisitor() = default;
};

class TypedExpression {
  public:
	virtual void visit(TypedExpressionVisitor& visitor) const = 0;
	virtual const std::string_view variantName() const = 0;
	virtual const Token& getToken() const = 0;
	virtual ~TypedExpression() = default;
};

class Variable : public TypedExpression {
  public:
	Token name;
	Token token;

	Variable(Token name,
	        Token token):
		name(std::move(name)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitVariableTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Variable"; }

	const Token& getToken() const override { return token; };
};
class Intrinsic : public TypedExpression {
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

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitIntrinsicTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Intrinsic"; }

	const Token& getToken() const override { return token; };
};
class Assign : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> lhs;
	Token assignmentOp;
	std::unique_ptr<TypedExpression> rhs;
	Token token;

	Assign(std::unique_ptr<TypedExpression> lhs,
	        Token assignmentOp,
	        std::unique_ptr<TypedExpression> rhs,
	        Token token):
		lhs(std::move(lhs)),
		assignmentOp(std::move(assignmentOp)),
		rhs(std::move(rhs)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitAssignTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Assign"; }

	const Token& getToken() const override { return token; };
};
class Binary : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> left;
	Token op;
	std::unique_ptr<TypedExpression> right;
	Token token;

	Binary(std::unique_ptr<TypedExpression> left,
	        Token op,
	        std::unique_ptr<TypedExpression> right,
	        Token token):
		left(std::move(left)),
		op(std::move(op)),
		right(std::move(right)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitBinaryTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Binary"; }

	const Token& getToken() const override { return token; };
};
class Call : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> callee;
	Token paren;
	std::vector<std::unique_ptr<TypedExpression>> arguments;
	Token token;

	Call(std::unique_ptr<TypedExpression> callee,
	        Token paren,
	        std::vector<std::unique_ptr<TypedExpression>> arguments,
	        Token token):
		callee(std::move(callee)),
		paren(std::move(paren)),
		arguments(std::move(arguments)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitCallTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Call"; }

	const Token& getToken() const override { return token; };
};
class IntrinsicCall : public TypedExpression {
  public:
	std::unique_ptr<Intrinsic> callee;
	Token paren;
	std::vector<std::unique_ptr<TypedExpression>> arguments;
	Token token;

	IntrinsicCall(std::unique_ptr<Intrinsic> callee,
	        Token paren,
	        std::vector<std::unique_ptr<TypedExpression>> arguments,
	        Token token):
		callee(std::move(callee)),
		paren(std::move(paren)),
		arguments(std::move(arguments)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitIntrinsicCallTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "IntrinsicCall"; }

	const Token& getToken() const override { return token; };
};
class Get : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> object;
	Token name;
	Token token;

	Get(std::unique_ptr<TypedExpression> object,
	        Token name,
	        Token token):
		object(std::move(object)),
		name(std::move(name)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitGetTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Get"; }

	const Token& getToken() const override { return token; };
};
class Grouping : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> expression;
	Token token;

	Grouping(std::unique_ptr<TypedExpression> expression,
	        Token token):
		expression(std::move(expression)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitGroupingTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Grouping"; }

	const Token& getToken() const override { return token; };
};
class Literal : public TypedExpression {
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

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitLiteralTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Literal"; }

	const Token& getToken() const override { return token; };
};
class Logical : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> left;
	Token op;
	std::unique_ptr<TypedExpression> right;
	Token token;

	Logical(std::unique_ptr<TypedExpression> left,
	        Token op,
	        std::unique_ptr<TypedExpression> right,
	        Token token):
		left(std::move(left)),
		op(std::move(op)),
		right(std::move(right)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitLogicalTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Logical"; }

	const Token& getToken() const override { return token; };
};
class Set : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> object;
	Token name;
	Token assignmentOp;
	std::unique_ptr<TypedExpression> value;
	Token token;

	Set(std::unique_ptr<TypedExpression> object,
	        Token name,
	        Token assignmentOp,
	        std::unique_ptr<TypedExpression> value,
	        Token token):
		object(std::move(object)),
		name(std::move(name)),
		assignmentOp(std::move(assignmentOp)),
		value(std::move(value)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitSetTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Set"; }

	const Token& getToken() const override { return token; };
};
class Unary : public TypedExpression {
  public:
	Token op;
	bool isPrefix;
	std::unique_ptr<TypedExpression> expr;
	Token token;

	Unary(Token op,
	        bool isPrefix,
	        std::unique_ptr<TypedExpression> expr,
	        Token token):
		op(std::move(op)),
		isPrefix(std::move(isPrefix)),
		expr(std::move(expr)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitUnaryTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Unary"; }

	const Token& getToken() const override { return token; };
};
class ArrayAccess : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> array;
	std::unique_ptr<TypedExpression> index;
	Token token;

	ArrayAccess(std::unique_ptr<TypedExpression> array,
	        std::unique_ptr<TypedExpression> index,
	        Token token):
		array(std::move(array)),
		index(std::move(index)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitArrayAccessTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "ArrayAccess"; }

	const Token& getToken() const override { return token; };
};
class ArrayType : public TypedExpression {
  public:
	bool isMutable;
	std::unique_ptr<TypedExpression> subType;
	Token token;

	ArrayType(bool isMutable,
	        std::unique_ptr<TypedExpression> subType,
	        Token token):
		isMutable(std::move(isMutable)),
		subType(std::move(subType)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitArrayTypeTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "ArrayType"; }

	const Token& getToken() const override { return token; };
};
class TupleType : public TypedExpression {
  public:
	bool isMutable;
	std::vector<std::unique_ptr<TypedExpression>> expressions;
	Token token;

	TupleType(bool isMutable,
	        std::vector<std::unique_ptr<TypedExpression>> expressions,
	        Token token):
		isMutable(std::move(isMutable)),
		expressions(std::move(expressions)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitTupleTypeTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "TupleType"; }

	const Token& getToken() const override { return token; };
};
class PointerType : public TypedExpression {
  public:
	bool isMutable;
	std::unique_ptr<TypedExpression> subtype;
	Token token;

	PointerType(bool isMutable,
	        std::unique_ptr<TypedExpression> subtype,
	        Token token):
		isMutable(std::move(isMutable)),
		subtype(std::move(subtype)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitPointerTypeTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "PointerType"; }

	const Token& getToken() const override { return token; };
};
class NamedType : public TypedExpression {
  public:
	Token name;
	bool isMutable;
	Token token;

	NamedType(Token name,
	        bool isMutable,
	        Token token):
		name(std::move(name)),
		isMutable(std::move(isMutable)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitNamedTypeTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "NamedType"; }

	const Token& getToken() const override { return token; };
};
class Cast : public TypedExpression {
  public:
	std::unique_ptr<TypedExpression> expression;
	std::unique_ptr<TypedExpression> type;
	Token token;

	Cast(std::unique_ptr<TypedExpression> expression,
	        std::unique_ptr<TypedExpression> type,
	        Token token):
		expression(std::move(expression)),
		type(std::move(type)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitCastTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Cast"; }

	const Token& getToken() const override { return token; };
};
class Parameter : public TypedExpression {
  public:
	Token name;
	std::unique_ptr<TypedExpression> type;
	Token token;

	Parameter(Token name,
	        std::unique_ptr<TypedExpression> type,
	        Token token):
		name(std::move(name)),
		type(std::move(type)),
		token(std::move(token)) {}

	void visit(TypedExpressionVisitor& visitor) const override {
		visitor.visitParameterTypedExpression(*this);
	}

	const std::string_view variantName() const override { return "Parameter"; }

	const Token& getToken() const override { return token; };
};

} // namespace ray::compiler::ast

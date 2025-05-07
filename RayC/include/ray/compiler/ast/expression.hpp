#pragma once
#include <memory>
#include <vector>
#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler::ast {

class Assign;
class Binary;
class Call;
class Get;
class Grouping;
class Literal;
class Logical;
class Set;
class Unary;
class Variable;
class Type;
class Parameter;

class ExpressionVisitor {
  public:
	virtual void visitAssignExpression(const Assign& value) = 0;
	virtual void visitBinaryExpression(const Binary& value) = 0;
	virtual void visitCallExpression(const Call& value) = 0;
	virtual void visitGetExpression(const Get& value) = 0;
	virtual void visitGroupingExpression(const Grouping& value) = 0;
	virtual void visitLiteralExpression(const Literal& value) = 0;
	virtual void visitLogicalExpression(const Logical& value) = 0;
	virtual void visitSetExpression(const Set& value) = 0;
	virtual void visitUnaryExpression(const Unary& value) = 0;
	virtual void visitVariableExpression(const Variable& value) = 0;
	virtual void visitTypeExpression(const Type& value) = 0;
	virtual void visitParameterExpression(const Parameter& value) = 0;
	virtual ~ExpressionVisitor() = default;
};

class Expression {
  public:
	virtual void visit(ExpressionVisitor& visitor) const = 0;
	virtual std::string variantName() const = 0;
	virtual ~Expression() = default;
};

class Assign : public Expression {
  public:
	Token name;
	std::unique_ptr<Expression> value;

	Assign(Token name,
	        std::unique_ptr<Expression> value)
	    : name(std::move(name)), value(std::move(value)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitAssignExpression(*this); }

	std::string variantName() const override { return "Assign"; }

};
class Binary : public Expression {
  public:
	std::unique_ptr<Expression> left;
	Token op;
	std::unique_ptr<Expression> right;

	Binary(std::unique_ptr<Expression> left,
	        Token op,
	        std::unique_ptr<Expression> right)
	    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitBinaryExpression(*this); }

	std::string variantName() const override { return "Binary"; }

};
class Call : public Expression {
  public:
	std::unique_ptr<Expression> callee;
	Token paren;
	std::vector<std::unique_ptr<Expression>> arguments;

	Call(std::unique_ptr<Expression> callee,
	        Token paren,
	        std::vector<std::unique_ptr<Expression>> arguments)
	    : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitCallExpression(*this); }

	std::string variantName() const override { return "Call"; }

};
class Get : public Expression {
  public:
	std::unique_ptr<Expression> object;
	Token name;

	Get(std::unique_ptr<Expression> object,
	        Token name)
	    : object(std::move(object)), name(std::move(name)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitGetExpression(*this); }

	std::string variantName() const override { return "Get"; }

};
class Grouping : public Expression {
  public:
	std::unique_ptr<Expression> expression;

	Grouping(std::unique_ptr<Expression> expression)
	    : expression(std::move(expression)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitGroupingExpression(*this); }

	std::string variantName() const override { return "Grouping"; }

};
class Literal : public Expression {
  public:
	Token kind;
	std::string value;

	Literal(Token kind,
	        std::string value)
	    : kind(std::move(kind)), value(std::move(value)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitLiteralExpression(*this); }

	std::string variantName() const override { return "Literal"; }

};
class Logical : public Expression {
  public:
	std::unique_ptr<Expression> left;
	Token op;
	std::unique_ptr<Expression> right;

	Logical(std::unique_ptr<Expression> left,
	        Token op,
	        std::unique_ptr<Expression> right)
	    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitLogicalExpression(*this); }

	std::string variantName() const override { return "Logical"; }

};
class Set : public Expression {
  public:
	std::unique_ptr<Expression> object;
	Token name;
	std::unique_ptr<Expression> value;

	Set(std::unique_ptr<Expression> object,
	        Token name,
	        std::unique_ptr<Expression> value)
	    : object(std::move(object)), name(std::move(name)), value(std::move(value)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitSetExpression(*this); }

	std::string variantName() const override { return "Set"; }

};
class Unary : public Expression {
  public:
	Token op;
	std::unique_ptr<Expression> right;

	Unary(Token op,
	        std::unique_ptr<Expression> right)
	    : op(std::move(op)), right(std::move(right)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitUnaryExpression(*this); }

	std::string variantName() const override { return "Unary"; }

};
class Variable : public Expression {
  public:
	Token name;

	Variable(Token name)
	    : name(std::move(name)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitVariableExpression(*this); }

	std::string variantName() const override { return "Variable"; }

};
class Type : public Expression {
  public:
	Token name;
	bool isConst;

	Type(Token name,
	        bool isConst)
	    : name(std::move(name)), isConst(std::move(isConst)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitTypeExpression(*this); }

	std::string variantName() const override { return "Type"; }

};
class Parameter : public Expression {
  public:
	Token name;
	Type type;

	Parameter(Token name,
	        Type type)
	    : name(std::move(name)), type(std::move(type)) {}

	void visit(ExpressionVisitor& visitor) const override { visitor.visitParameterExpression(*this); }

	std::string variantName() const override { return "Parameter"; }

};

} // namespace ray::compiler::ast

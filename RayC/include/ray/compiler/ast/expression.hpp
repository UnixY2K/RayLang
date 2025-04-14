#pragma once
#include <any>
#include <memory>
#include <vector>
#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler::ast {

class Expression {
  public:
	virtual ~Expression() = default;
};

class Ternary : public Expression {
  public:
	std::unique_ptr<Expression> cond;
	std::unique_ptr<Expression> left;
	std::unique_ptr<Expression> right;

	Ternary(std::unique_ptr<Expression> cond,
	        std::unique_ptr<Expression> left,
	        std::unique_ptr<Expression> right)
	    : cond(std::move(cond)), left(std::move(left)), right(std::move(right)) {}

};
class Assign : public Expression {
  public:
	Token name;
	std::unique_ptr<Expression> value;

	Assign(Token name,
	        std::unique_ptr<Expression> value)
	    : name(std::move(name)), value(std::move(value)) {}

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

};
class Get : public Expression {
  public:
	std::unique_ptr<Expression> object;
	Token name;

	Get(std::unique_ptr<Expression> object,
	        Token name)
	    : object(std::move(object)), name(std::move(name)) {}

};
class Grouping : public Expression {
  public:
	std::unique_ptr<Expression> expression;

	Grouping(std::unique_ptr<Expression> expression)
	    : expression(std::move(expression)) {}

};
class Literal : public Expression {
  public:
	std::any value;

	Literal(std::any value)
	    : value(std::move(value)) {}

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

};
class Unary : public Expression {
  public:
	Token op;
	std::unique_ptr<Expression> right;

	Unary(Token op,
	        std::unique_ptr<Expression> right)
	    : op(std::move(op)), right(std::move(right)) {}

};
class Variable : public Expression {
  public:
	Token name;

	Variable(Token name)
	    : name(std::move(name)) {}

};
class Type : public Expression {
  public:
	Token type;

	Type(Token type)
	    : type(std::move(type)) {}

};
class Parameter : public Expression {
  public:
	Token name;
	Type type;

	Parameter(Token name,
	        Type type)
	    : name(std::move(name)), type(std::move(type)) {}

};


class ExpressionVisitor {
  public:
	virtual void visitTernaryExpression(Ternary& value) = 0;
	virtual void visitAssignExpression(Assign& value) = 0;
	virtual void visitBinaryExpression(Binary& value) = 0;
	virtual void visitCallExpression(Call& value) = 0;
	virtual void visitGetExpression(Get& value) = 0;
	virtual void visitGroupingExpression(Grouping& value) = 0;
	virtual void visitLiteralExpression(Literal& value) = 0;
	virtual void visitLogicalExpression(Logical& value) = 0;
	virtual void visitSetExpression(Set& value) = 0;
	virtual void visitUnaryExpression(Unary& value) = 0;
	virtual void visitVariableExpression(Variable& value) = 0;
	virtual void visitTypeExpression(Type& value) = 0;
	virtual void visitParameterExpression(Parameter& value) = 0;
};

} // namespace ray::compiler::ast

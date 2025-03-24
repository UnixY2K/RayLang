#pragma once
#include <memory>
#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler::ast {

template <class T> class ExpressionVisitor;

template <class T> class Expression {
  public:
	virtual T accept(ExpressionVisitor<T> &visitor) = 0;
};

template <class T> class Ternary : Expression<T> {
  public:
	std::unique_ptr<Expression<T>> cond;
	std::unique_ptr<Expression<T>> left;
	std::unique_ptr<Expression<T>> right;

	Ternary(std::unique_ptr<Expression<T>> cond,
	        std::unique_ptr<Expression<T>> left,
	        std::unique_ptr<Expression<T>> right)
	    : cond(std::move(cond)), left(std::move(left)), right(std::move(right)) {}

	T accept(ExpressionVisitor<T> &visitor) override {
		return visitor.visitTernaryExpression(this);
	}
};

template <class T> class Variable : Expression<T> {
  public:
	std::unique_ptr<Token> name;

	Variable(std::unique_ptr<Token> name)
	    : name(std::move(name)) {}

	T accept(ExpressionVisitor<T> &visitor) override {
		return visitor.visitVariableExpression(this);
	}
};


template <class T> class ExpressionVisitor {
	virtual T visitTernaryExpression(Ternary<T> &ternary) = 0;
	virtual T visitVariableExpression(Variable<T> &ternary) = 0;
};

} // namespace ray::compiler::ast

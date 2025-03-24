#pragma once
#include <memory>

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
	    : cond(cond), left(left), right(right) {}

	T accept(ExpressionVisitor<T> &visitor) override {
		return visitor.visitTernaryExpression(this);
	}
};

template <class T> class ExpressionVisitor {
	virtual T visitTernaryExpression(Ternary<T> &ternary) = 0;
};

} // namespace ray::compiler::ast

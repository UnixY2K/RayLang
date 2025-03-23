#pragma once
#include <memory>

namespace ray::compiler::ast {

template <class T> class ExprVisitor;

template <class T> class Expression {
  public:
	virtual T accept(ExprVisitor<T> &visitor) = 0;
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

	T accept(ExprVisitor<T> &visitor) override {
		return visitor.visitTernaryExpr(this);
	}
};

template <class T> class ExprVisitor {
	virtual T visitTernaryExpr(Ternary<T> &ternary) = 0;
};
} // namespace ray::compiler::ast

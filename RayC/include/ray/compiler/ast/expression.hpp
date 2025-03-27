#pragma once
#include <memory>
#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler::ast {

class Expression {
  public:
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
class Variable : public Expression {
  public:
	std::unique_ptr<Token> name;

	Variable(std::unique_ptr<Token> name)
	    : name(std::move(name)) {}

};

} // namespace ray::compiler::ast

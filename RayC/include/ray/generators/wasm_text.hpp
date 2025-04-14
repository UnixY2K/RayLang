#pragma once
#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::generator {

class WASMTextGenerator : public ast::ExpressionVisitor,
                          public ast::StatementVisitor {
  public:
	WASMTextGenerator(const WASMTextGenerator &) = default;
	WASMTextGenerator(WASMTextGenerator &&) = default;
	WASMTextGenerator &operator=(const WASMTextGenerator &) = default;
	WASMTextGenerator &operator=(WASMTextGenerator &&) = default;
};

} // namespace ray::compiler::generator

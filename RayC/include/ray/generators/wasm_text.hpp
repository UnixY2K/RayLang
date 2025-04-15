#pragma once
#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::generator {

class WASMTextGenerator : public ast::ExpressionVisitor,
                          public ast::StatementVisitor {
  public:

};

} // namespace ray::compiler::generator

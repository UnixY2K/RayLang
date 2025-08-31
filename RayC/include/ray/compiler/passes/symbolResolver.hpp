#pragma once

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::passes {

class symbolResolver : public ast::StatementVisitor,
                       public ast::ExpressionVisitor {
						
					   };

} // namespace ray::compiler::passes
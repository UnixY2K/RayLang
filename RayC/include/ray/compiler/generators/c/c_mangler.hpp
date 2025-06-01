#pragma once
#include <optional>
#include <string>
#include <string_view>

#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>

namespace ray::compiler::generator::c {
class NameMangler {
  public:
	std::string mangleFunction(std::string_view module,
	                           std::string_view namespacePath,
	                           const ast::Function &function,
							std::optional<directive::LinkageDirective> &linkageDirective);
	std::string mangleStruct(std::string_view module,
	                         std::string_view namespacePath,
	                         const ast::Struct &structDefinition);
};
} // namespace ray::compiler::generator::c

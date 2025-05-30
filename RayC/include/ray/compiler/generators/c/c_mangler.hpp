#pragma once
#include <string>
#include <string_view>

#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::generator::c {
class NameMangler {

	std::string mangleFunction(std::string_view module,
	                           std::string_view namespacePath,
	                           const ast::Function &function);
	std::string mangleStruct(std::string_view module,
	                         std::string_view namespacePath,
	                         const ast::Struct &structDefinition);
};
} // namespace ray::compiler::generator::c

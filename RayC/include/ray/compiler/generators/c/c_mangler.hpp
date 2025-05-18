#pragma once
#include <string>
#include <string_view>

#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::generator::c {
class NameMangler {

	std::string mangleFunction(std::string_view module,
	                           std::string namespacePath,
	                           const ast::Function &function);
};
} // namespace ray::compiler::generators::c

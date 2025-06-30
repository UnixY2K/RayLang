#pragma once

#include <optional>
#include <string>

#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::lang {

struct Symbol {
	enum class SymbolType { Function, Struct, Variable, Parameter };
	std::string name;
	std::string mangledName;
	SymbolType type;
	std::string scope;
	bool internal = false;
	std::optional<const ast::Statement *> object;
};
} // namespace ray::compiler::lang
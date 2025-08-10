#pragma once

#include <string>

#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::lang {

struct Symbol {
	enum class SymbolType { Function, Struct, Variable, Parameter };
	std::string name;
	std::string mangledName;
	lang::Type innerType;
	SymbolType type;
	bool internal = false;
};
} // namespace ray::compiler::lang
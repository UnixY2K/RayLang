#pragma once

#include <cstddef>
#include <string>

#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

struct Symbol {
	enum class SymbolType { Function, Struct, Variable, Parameter };
	size_t symbolId;
	std::string name;
	std::string mangledName;
	lang::Type innerType;
	SymbolType type;
	bool internal = false;
};
} // namespace ray::compiler::lang
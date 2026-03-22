#pragma once

#include <cstddef>
#include <string>

#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

struct Symbol {
	enum class SymbolType { Function, Struct, Variable, Parameter, Unknown };
	size_t symbolId;
	std::string name;
	std::string mangledName;
	lang::Type innerType;
	SymbolType type;
	bool internal = false;

	static constexpr Symbol defineUnknownSymbol() {
		return Symbol{.symbolId = 0,
		              .name = "%<unknown-symbol>%",
		              .mangledName = "%<unknown-symbol>%",
		              .innerType = lang::Type::defineUnknownType(),
		              .type = Symbol::SymbolType::Unknown,
		              .internal = true};
	}
};

} // namespace ray::compiler::lang
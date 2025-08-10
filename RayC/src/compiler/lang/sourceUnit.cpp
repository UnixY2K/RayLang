#include "ray/compiler/lang/symbol.hpp"
#include <cstddef>
#include <optional>
#include <vector>

#include <msgpack.hpp>

#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

bool lang::Scope::defineStruct(Type type) {
	// check if is a new struct
	if (variables.contains(type.name)) {
		const auto &symbol = variables.at(type.name);
		if (!symbol.innerType.isScalar() &&
		    symbol.innerType.calculatedSize > 0) {
			// we cannot redefine a known non scalar type
			return false;
		}
	}
	// non found, redefine it
	Symbol structSymbol{
	    .name = type.name,
	    .mangledName = "",
	    .innerType = type,
	    .type = Symbol::SymbolType::Struct,
	    .internal = false,
	};
	variables[type.name] = structSymbol;
	return true;
}

bool lang::Scope::defineFunction(FunctionDeclaration declaration) {

	if (variables.contains(declaration.name)) {
		return false;
	}

	if (!functions.contains(declaration.name)) {
		functions[declaration.name] = std::vector<FunctionDeclaration>{};
	}

	auto &declarations = functions.at(declaration.name);
	if (!declarations.empty() && declarations[0].signature.returnType !=
	                                 declaration.signature.returnType) {
		// this should be probably an enum with an error code
		return false;
	}

	for (auto &declaration : declarations) {
		bool matching = false;
		if (declaration.signature.parameters.size() ==
		    declaration.signature.parameters.size()) {
			for (size_t paramIndex = 0;
			     paramIndex < declaration.signature.parameters.size();
			     paramIndex++) {
				if (declaration.signature.parameters[paramIndex] !=
				    declaration.signature.parameters[paramIndex]) {
					matching = false;
					break;
				}
			}
		}
		if (matching) {
			return false;
		}
	}

	declarations.push_back(declaration);

	return true;
}

bool lang::Scope::defineLocalVariable(const lang::Symbol symbol) {
	if (variables.contains(symbol.name)) {
		return false;
	}
	variables[symbol.name] = symbol;
	return true;
}

std::optional<lang::Type>
SourceUnit::findStructType(const std::string &typeName) const {
	if (rootScope.variables.contains(typeName)) {
		return rootScope.variables.at(typeName).innerType;
	}

	return {};
}

} // namespace ray::compiler::lang
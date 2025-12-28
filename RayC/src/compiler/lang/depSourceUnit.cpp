#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include <ray/compiler/lang/depSourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

bool DepScope::declareStruct(Type type, const std::string_view mangledName) {
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
	    .mangledName = std::string(mangledName),
	    .innerType = type,
	    .type = Symbol::SymbolType::Struct,
	    .internal = false,
	};
	variables[type.name] = structSymbol;
	return true;
}

bool DepScope::defineFunction(FunctionDeclaration declaration) {

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

bool DepScope::defineLocalVariable(const Symbol symbol) {
	if (variables.contains(symbol.name)) {
		return false;
	}
	variables[symbol.name] = symbol;
	return true;
}

// TODO: implement heterogeneous search for std::unordered_map
// to avoid copying a string on search
const std::optional<const Symbol>
DepScope::findVariable(const std::string_view name) const {
	const std::string key(name);
	if (variables.contains(key)) {
		return variables.at(key);
	}
	return std::nullopt;
}

const std::optional<const std::vector<FunctionDeclaration>>
DepScope::findFunctionDeclaration(const std::string_view name) const {
	const std::string key(name);
	if (functions.contains(key)) {
		return functions.at(key);
	}
	return std::nullopt;
}

const std::optional<const Struct>
DepScope::findStruct(const std::string_view name) const {
	const std::string key(name);
	if (structs.contains(key)) {
		return structs.at(key);
	}
	return std::nullopt;
}

std::optional<Type>
DepSourceUnit::findStructType(const std::string &typeName) const {
	if (depRootScope.variables.contains(typeName)) {
		return depRootScope.variables.at(typeName).innerType;
	}

	return {};
}

DepScope &DepScope::makeChildScope() {
	innerScopes.push_back(DepScope(*this));
	return *innerScopes.back().get();
}

} // namespace ray::compiler::lang
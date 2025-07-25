#include <cstddef>
#include <istream>
#include <optional>
#include <sstream>
#include <vector>

#include <msgpack.hpp>

#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

bool lang::Scope::defineStruct(Type type) {
	// check if is a new struct
	if (variables.contains(type.name)) {
		const auto &varType = variables.at(type.name);
		if (!varType.isScalar() && varType.calculatedSize > 0) {
			// we cannot redefine a known non scalar type
			return false;
		}
	}
	// non found, redefine it
	variables[type.name] = type;
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

bool lang::Scope::defineLocalVariable(const std::string_view name, const lang::Type type) {
	std::string key(name);
	if (variables.contains(key)) {
		return false;
	}
	variables[key] = type;
	return true;
}

std::optional<lang::Type>
SourceUnit::findStructType(const std::string &typeName) const {
	for (const auto &scope : scopes) {
		if (scope.variables.contains(typeName)) {
			return scope.variables.at(typeName);
		}
	}
	return {};
}

void S1SourceUnit::clear() {
	structDefinitions.clear();
	structDeclarations.clear();

	functionDeclarations.clear();

	rootScope.clear();
}

std::string S1SourceUnit::exportSourceUnit() const {
	std::stringstream result;
	msgpack::pack(result, *this);

	result.seekg(0);

	return result.str();
}

S1SourceUnit S1SourceUnit::importSourceUnit(std::stringstream &stream) {
	S1SourceUnit result;
	std::string data = stream.str();

	msgpack::object_handle oh = msgpack::unpack(data.data(), data.size());
	msgpack::object obj = oh.get();

	obj.convert(result);
	return result;
}
} // namespace ray::compiler::lang
#include <istream>
#include <optional>
#include <sstream>
#include <string_view>
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
	functionDeclarations.clear();

	structDeclarations.clear();
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
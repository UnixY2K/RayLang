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

std::optional<lang::Type>
SourceUnit::findStructType(const std::string &typeName) const {
	return globalStructTypes.contains(typeName)
	           ? std::optional<lang::Type>(globalStructTypes.at(typeName))
	           : std::nullopt;
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
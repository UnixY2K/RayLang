#include <sstream>
#include <vector>

#include <msgpack.hpp>

#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/structDefinition.hpp>

namespace ray::compiler::lang {
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

S1SourceUnit S1SourceUnit::importSourceUnit() {
	S1SourceUnit result;

	return result;
}
} // namespace ray::compiler::lang
#include <sstream>

#include <ray/compiler/lang/sourceUnit.hpp>

namespace ray::compiler::lang {
void S1SourceUnit::clear() {
	functionDeclarations.clear();
	functionDefinitions.clear();

	structDeclarations.clear();
	structDefinitions.clear();
}

std::string S1SourceUnit::exportSourceUnit() const {
	std::stringstream result;

	return result.str();
}

S1SourceUnit S1SourceUnit::importSourceUnit() {
	S1SourceUnit result;

	return result;
}
} // namespace ray::compiler::lang
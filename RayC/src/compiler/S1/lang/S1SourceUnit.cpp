#include <ray/compiler/S1/lang/S1SourceUnit.hpp>

namespace ray::compiler::S1::lang {

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

} // namespace ray::compiler::S1::lang

#pragma once
#include <sstream>
#include <string>

#include <msgpack.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/type.hpp>
#include <unordered_map>

namespace ray::compiler::lang {
class SourceUnit {
  public:
	std::vector<StructDeclaration> structDeclarations;
	std::vector<Struct> structDefinitions;

	std::vector<FunctionDeclaration> functionDeclarations;
	std::vector<FunctionDefinition> functionDefinitions;

	std::unordered_map<std::string, lang::Type> globalStructTypes;

	std::optional<lang::Type> findStructType(const std::string &typeName) const;
};

class S1SourceUnit {
  public:
	std::vector<S1StructDeclaration> structDeclarations;
	std::vector<S1StructDefinition> structDefinitions;

	std::vector<S1FunctionDeclaration> functionDeclarations;

	void clear();

	std::string exportSourceUnit() const;
	static S1SourceUnit importSourceUnit(std::stringstream &stream);

	MSGPACK_DEFINE(structDeclarations, structDefinitions, functionDeclarations);
};
} // namespace ray::compiler::lang
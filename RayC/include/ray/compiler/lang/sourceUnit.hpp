#pragma once
#include <sstream>
#include <string>
#include <unordered_map>

#include <msgpack.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {
class Scope {
  public:
	std::unordered_map<std::string, lang::Type> variables;

	bool defineStruct(Type type);
};
class SourceUnit {
  public:
	std::vector<Scope> scopes = {Scope{}};
	// all functions and structs have a declaration vector
	// that comes from the previous top level resolver
	std::vector<StructDeclaration> structDeclarations;
	std::vector<FunctionDeclaration> functionDeclarations;

	std::vector<Struct> structDefinitions;
	std::vector<FunctionDefinition> functionDefinitions;

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
#pragma once
#include <string>

#include <msgpack.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/structDefinition.hpp>

namespace ray::compiler::lang {
class SourceUnit {
  public:
	std::vector<StructDefinition> structDeclarations;
	std::vector<FunctionDefinition> functionDeclarations;

	std::vector<StructDefinition> structDefinitions;
	std::vector<FunctionDefinition> functionDefinitions;
};

class S1SourceUnit {
  public:
	std::vector<S1StructDeclaration> structDeclarations;
	std::vector<S1StructDefinition> structDefinitions;

	std::vector<S1FunctionDeclaration> functionDeclarations;


	void clear();

	std::string exportSourceUnit() const;
	static S1SourceUnit importSourceUnit();

	MSGPACK_DEFINE(structDeclarations, structDefinitions, functionDeclarations);
};
} // namespace ray::compiler::lang
#pragma once
#include <string>

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
	std::vector<S1StructDefinition> structDeclarations;
	std::vector<S1FunctionDefinition> functionDeclarations;

	std::vector<S1StructDefinition> structDefinitions;
	std::vector<S1FunctionDefinition> functionDefinitions;

	void clear();

	std::string exportSourceUnit() const;
	static S1SourceUnit importSourceUnit();
};
} // namespace ray::compiler::lang
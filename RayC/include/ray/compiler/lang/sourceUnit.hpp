#pragma once
#include <string>
#include <vector>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/structDefinition.hpp>

namespace ray::compiler::lang {
class SourceUnit {
  public:
	std::vector<std::string> requiredFiles;

	std::vector<StructDefinition> structDeclarations;
	std::vector<FunctionDefinition> functionDeclarations;

	std::vector<StructDefinition> structDefinitions;
	std::vector<FunctionDefinition> functionDefinitions;
};
} // namespace ray::compiler::lang
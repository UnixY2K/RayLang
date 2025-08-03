#pragma once
#include <optional>
#include <string>
#include <unordered_map>

#include <ray/util/copy_ptr.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {
class Scope {
  public:
	std::unordered_map<std::string, lang::Type> variables;
	std::unordered_map<std::string, std::vector<FunctionDeclaration>> functions;

	bool defineStruct(Type type);
	bool defineFunction(FunctionDeclaration declaration);
	bool defineLocalVariable(const std::string_view name,
	                         const lang::Type type);
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

} // namespace ray::compiler::lang
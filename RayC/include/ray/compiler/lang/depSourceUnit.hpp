#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <ray/util/copy_ptr.hpp>
#include <ray/util/soft_reference.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

class DepScope;
class DepScope {
  public:
	std::optional<DepScope *> parentScope;
	std::string scopeName;
	std::vector<util::copy_ptr<DepScope>> innerScopes;

	std::unordered_map<std::string, lang::Symbol> variables;
	std::unordered_map<std::string, std::vector<FunctionDeclaration>> functions;
	std::unordered_map<std::string, Struct> structs;

	DepScope(std::optional<DepScope *> parentScope = std::nullopt)
	    : parentScope(parentScope) {}

	bool declareStruct(Type type, const std::string_view mangledName);
	bool defineFunction(FunctionDeclaration declaration);
	bool defineLocalVariable(const lang::Symbol symbol);

	const std::optional<const Symbol>
	findVariable(const std::string_view name) const;

	const std::optional<const std::vector<FunctionDeclaration>>
	findFunctionDeclaration(const std::string_view name) const;

	const std::optional<const Struct>
	findStruct(const std::string_view name) const;

	DepScope &makeChildScope();
};

class DepSourceUnit {
	size_t nextId = 1;

	std::unordered_map<size_t, Symbol> variables;
	std::unordered_map<size_t, FunctionDeclaration> functions;
	std::unordered_map<size_t, Struct> structs;

  public:
	DepScope depRootScope;
	Scope rootScope;
	// all functions and structs have a declaration vector
	// that comes from the previous top level resolver
	std::vector<StructDeclaration> structDeclarations;
	std::vector<FunctionDeclaration> functionDeclarations;

	std::vector<Struct> structDefinitions;
	std::vector<FunctionDefinition> functionDefinitions;

	std::optional<lang::Type> findStructType(const std::string &typeName) const;
};

} // namespace ray::compiler::lang
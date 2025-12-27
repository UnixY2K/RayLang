#pragma once
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <ray/util/copy_ptr.hpp>
#include <ray/util/soft_reference.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

class ScopeContext;
class ScopeContext {
	std::string scopeName;
	std::optional<std::reference_wrapper<ScopeContext>> parentScope;
	std::vector<util::copy_ptr<ScopeContext>> innerScopes;

	std::unordered_map<std::string, util::soft_reference<Symbol>> variables;
	std::unordered_map<std::string,
	                   std::vector<util::soft_reference<FunctionDeclaration>>>
	    functions;
	std::unordered_map<std::string, util::soft_reference<Struct>> structs;

  public:
	ScopeContext(std::optional<std::reference_wrapper<ScopeContext>>
	                 parentScope = std::nullopt)
	    : parentScope(parentScope) {}

	bool declareStruct(Type type, const std::string_view mangledName);
	bool defineFunction(FunctionDeclaration declaration);
	bool defineLocalVariable(const Symbol symbol);

	const std::optional<const util::soft_reference<Symbol>>
	findVariable(const std::string_view name) const;

	const std::optional<
	    const std::vector<util::soft_reference<FunctionDeclaration>>>
	findFunctionDeclaration(const std::string_view name) const;

	const std::optional<const util::soft_reference<Struct>>
	findStruct(const std::string_view name) const;

	ScopeContext &makeChildScope();
};

class Scope;
class Scope {
  public:
	std::optional<Scope *> parentScope;
	std::string scopeName;
	std::vector<util::copy_ptr<Scope>> innerScopes;

	std::unordered_map<std::string, lang::Symbol> variables;
	std::unordered_map<std::string, std::vector<FunctionDeclaration>> functions;
	std::unordered_map<std::string, Struct> structs;

	Scope(std::optional<Scope *> parentScope = std::nullopt)
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

	Scope &makeChildScope();
};

class SourceUnit {
  public:
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
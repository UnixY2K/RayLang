#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <ray/util/copy_ptr.hpp>
#include <ray/util/soft_reference.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {
class Scope;
class Scope {
	std::string scopeName;
	std::optional<std::reference_wrapper<Scope>> parentScope;
	std::vector<util::copy_ptr<Scope>> innerScopes;

	std::unordered_map<std::string, util::soft_reference<Symbol>> variables;
	std::unordered_map<std::string,
	                   std::vector<util::soft_reference<FunctionDeclaration>>>
	    functions;
	std::unordered_map<std::string, util::soft_reference<Struct>> structs;

  public:
	Scope(
	    std::optional<std::reference_wrapper<Scope>> parentScope = std::nullopt)
	    : parentScope(parentScope) {}

	bool bindStruct(std::string_view name, util::soft_reference<Struct>& structRef);

	bool declareStruct(const std::string_view name);
	bool defineFunction(FunctionDeclaration declaration);
	bool defineLocalVariable(const Symbol symbol);

	const std::optional<const util::soft_reference<Symbol>>
	findVariable(const std::string_view name) const;

	const std::optional<
	    const std::vector<util::soft_reference<FunctionDeclaration>>>
	findFunctionDeclaration(const std::string_view name) const;

	const std::optional<const util::soft_reference<Struct>>
	findStruct(const std::string_view name) const;

	Scope &makeChildScope();
	std::optional<std::reference_wrapper<Scope>> getParentScope() const {
		return parentScope;
	}
};

} // namespace ray::compiler::lang
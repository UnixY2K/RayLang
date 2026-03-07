#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::lang {
class SourceUnit {
	size_t nextId = 1;

	std::unordered_map<size_t, Symbol> variables;
	std::unordered_map<size_t, FunctionDeclaration> functions;
	std::unordered_map<size_t, Struct> structs;

  public:
	Scope rootScope;

	[[nodiscard("must check struct binding result")]]
	bool bindStruct(const Struct &structobj, Scope &scope);
	[[nodiscard("must check declaration result")]]
	bool declareFunction(const FunctionDeclaration &functionDeclaration,
	                     Scope &scope);
	[[nodiscard("must check declaration result")]]
	bool declareLocalVariable(const Symbol symbol, Scope &scope);

	std::optional<std::reference_wrapper<Struct>>
	findStruct(const std::string_view structName,
	           const Scope &currentScope) const;

	const std::unordered_map<size_t, FunctionDeclaration> &
	getFunctions() const {
		return functions;
	}
	const std::unordered_map<size_t, Struct> &getStructs() const {
		return structs;
	}
};
} // namespace ray::compiler::lang
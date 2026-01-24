#pragma once
#include <functional>
#include <optional>

#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/util/soft_reference.hpp>
#include <string_view>

namespace ray::compiler::lang {
class SourceUnit {
	size_t nextId = 1;

	std::unordered_map<size_t, Symbol> variables;
	std::unordered_map<size_t, FunctionDeclaration> functions;
	std::unordered_map<size_t, Struct> structs;

  public:
	Scope rootScope;

	bool bindStruct(const Struct &structobj, Scope &scope);

	std::optional<std::reference_wrapper<Struct>>
	findStruct(std::string_view structName, Scope &currentScope);
};
} // namespace ray::compiler::lang
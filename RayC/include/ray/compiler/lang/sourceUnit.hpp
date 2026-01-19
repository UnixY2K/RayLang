#pragma once
#include <optional>

#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::lang {
class SourceUnit {
	size_t nextId = 1;

	std::unordered_map<size_t, Symbol> variables;
	std::unordered_map<size_t, FunctionDeclaration> functions;
	std::unordered_map<size_t, Struct> structs;

  public:
	Scope rootScope;

	std::optional<util::soft_reference<Struct>>
	declareStruct(const Struct &structobj);
};
} // namespace ray::compiler::lang
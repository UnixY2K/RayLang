#pragma once
#include <functional>
#include <string>

#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::lang {
class StructDefinition {
  public:
	std::string name;
	std::string mangledName;
	std::reference_wrapper<const ast::Struct> structObj;
};
} // namespace ray::compiler::lang
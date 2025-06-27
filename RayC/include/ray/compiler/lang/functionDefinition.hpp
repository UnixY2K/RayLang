#pragma once
#include <functional>
#include <string>

#include <ray/compiler/ast/statement.hpp>
namespace ray::compiler::lang {

class FunctionDefinition {

  public:
	std::string name;
	std::string mangledName;
	std::reference_wrapper<const ast::Function> function;
};
} // namespace ray::compiler::lang

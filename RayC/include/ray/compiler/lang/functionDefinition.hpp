#pragma once
#include <functional>
#include <string>

#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

class FunctionDefinition {

  public:
	std::string name;
	std::string mangledName;
	std::reference_wrapper<const ast::Function> function;
	Type returnType;
};

class S1FunctionDefinition {

  public:
	std::string name;
	std::string mangledName;
	std::string returnType;
};
} // namespace ray::compiler::lang

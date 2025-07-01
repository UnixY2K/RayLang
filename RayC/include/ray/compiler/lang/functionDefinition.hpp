#pragma once
#include <functional>
#include <string>

#include <msgpack.hpp>

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

class S1FunctionDeclaration {

  public:
	std::string name;
	std::string mangledName;
	std::string returnType;

	MSGPACK_DEFINE(name, mangledName, returnType);
};
} // namespace ray::compiler::lang

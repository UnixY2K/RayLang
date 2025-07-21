#pragma once
#include <functional>
#include <string>

#include <msgpack.hpp>

#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/type.hpp>
#include <vector>

namespace ray::compiler::lang {

class FunctionParameter {
  public:
	std::string name;
	Type parameterType;

	bool operator==(const FunctionParameter &other) const {
		return name == other.name && parameterType == other.parameterType;
	}
};

struct FunctionSignature {
	Type returnType;
	std::vector<FunctionParameter> parameters;
};

class FunctionDeclaration {
  public:
	std::string name;
	std::string mangledName;
	bool publicVisibility;
	FunctionSignature signature;
};

class FunctionDefinition {
  public:
	std::string name;
	std::string mangledName;
	std::reference_wrapper<const ast::Function> function;
	FunctionSignature signature;
};

class S1FunctionParameter {
  public:
	std::string name;
	std::string type;

	MSGPACK_DEFINE(name, type);
};

class S1FunctionDeclaration {

  public:
	std::string name;
	std::string mangledName;
	std::string returnType;

	std::vector<S1FunctionParameter> parameters;

	MSGPACK_DEFINE(name, mangledName, returnType, parameters);
};
} // namespace ray::compiler::lang

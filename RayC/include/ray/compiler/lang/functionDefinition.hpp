#pragma once
#include "ray/util/copy_ptr.hpp"
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

	Type getFunctionType() const {
		std::vector<util::copy_ptr<Type>> signature;
		for (const auto &parameter : parameters) {
			signature.push_back(parameter.parameterType);
		}
		return lang::Type::defineFunctionType(returnType, signature);
	}

	Type getOverloadedFunctionType() const {
		return lang::Type::defineOverloadedFunctionType(returnType);
	}
};

struct FunctionDeclaration {
	std::string name;
	std::string mangledName;
	bool publicVisibility;
	FunctionSignature signature;
};

struct FunctionDefinition {
	FunctionDeclaration declaration;
	std::reference_wrapper<const ast::Function> function;
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

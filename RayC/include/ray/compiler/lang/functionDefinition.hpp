#pragma once
#include <functional>
#include <string>

#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>
#include <ray/util/copy_ptr.hpp>
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

	Type getFunctionType(const environment::DataModel &dataModel) const {
		std::vector<util::copy_ptr<Type>> signature;
		for (const auto &parameter : parameters) {
			signature.push_back(parameter.parameterType);
		}
		return dataModel.defineFunctionType(returnType, signature);
	}

	Type
	getOverloadedFunctionType(const environment::DataModel &dataModel) const {
		return dataModel.defineOverloadedFunctionType(returnType);
	}
};

struct FunctionDeclaration {
	size_t functionID;
	std::string name;
	std::string mangledName;
	bool publicVisibility;
	FunctionSignature signature;
};

struct FunctionDefinition {
	FunctionDeclaration declaration;
	std::reference_wrapper<const syntax::ast::Function> function;
};

} // namespace ray::compiler::lang

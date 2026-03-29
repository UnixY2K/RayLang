#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {
class MethodParameter {
  public:
	std::string name;
	Type parameterType;

	bool operator==(const MethodParameter &other) const {
		return name == other.name && parameterType == other.parameterType;
	}
};

class MethodSignature {
  public:
	Type returnType;
	std::vector<MethodParameter> parameters;

	Type getMethodType(const environment::DataModel &dataModel) const;

	Type getOverloadedMethodType(const environment::DataModel &dataModel) const;
};

class Method {
  public:
	size_t methodID;
	std::string name;
	std::string mangledName;
	bool publicVisibility = false;
	MethodSignature signature;
};

class Trait {
  public:
	size_t traitID;
	std::string name;
	std::string mangledName;
	std::vector<Method> methods;
};
} // namespace ray::compiler::lang
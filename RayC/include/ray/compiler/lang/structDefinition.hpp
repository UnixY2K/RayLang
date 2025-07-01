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
class S1StructMember {
	std::string name;
	std::string type;
};
class S1StructDefinition {
  public:
	std::string name;
	std::string mangledName;
	bool isDefinition;
	std::vector<S1StructMember> members;
};
} // namespace ray::compiler::lang
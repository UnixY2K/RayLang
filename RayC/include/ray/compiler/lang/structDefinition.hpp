#pragma once
#include <cstddef>
#include <functional>
#include <string>

#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {
class StructDeclaration {
  public:
	std::string name;
	std::string mangledName;
};
class StructMember {
  public:
	bool publicAccess;
	std::string name;
	Type type;

	size_t calculateSize() const { return type.calculatedSize; }
};
class Struct {
  public:
	std::string name;
	std::string mangledName;
	std::vector<StructMember> members;
	std::reference_wrapper<const ast::Struct> structObj;
};

} // namespace ray::compiler::lang
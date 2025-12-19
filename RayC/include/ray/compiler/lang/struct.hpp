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
	bool publicVisibility = false;
	bool isMutable = false;
	std::string name;
	size_t typeId;
	Type type;

	size_t calculateSize() const { return type.calculatedSize; }
};
class Struct {
  public:
	size_t structID;
	std::string name;
	std::string mangledName;
	std::vector<StructMember> members;
	// TODO: decouple AST from type data
	std::reference_wrapper<const ast::Struct> structObj;
};

} // namespace ray::compiler::lang
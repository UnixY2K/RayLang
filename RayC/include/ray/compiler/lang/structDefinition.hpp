#pragma once
#include <cstddef>
#include <functional>
#include <string>

#include <msgpack.hpp>

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
	std::vector<Type> members;
	std::reference_wrapper<const ast::Struct> structObj;
};
class S1StructMember {
  public:
	std::string name;
	std::string type;

	MSGPACK_DEFINE(name, type);
};
class S1StructDefinition {
  public:
	std::string name;
	std::string mangledName;
	std::vector<S1StructMember> members;

	MSGPACK_DEFINE(name, mangledName, members);
};
class S1StructDeclaration {
  public:
	std::string name;
	std::string mangledName;

	MSGPACK_DEFINE(name, mangledName);
};
} // namespace ray::compiler::lang
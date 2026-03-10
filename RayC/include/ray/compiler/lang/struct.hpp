#pragma once
#include <cstddef>
#include <string>

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
	Type type;

	size_t calculateSize() const;
};
class Struct {
  public:
	bool opaque;
	size_t structID;
	std::string name;
	std::string mangledName;
	std::vector<StructMember> members;
};

} // namespace ray::compiler::lang
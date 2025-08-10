#pragma once
#include <optional>
#include <string>
#include <vector>

#include <msgpack.hpp>

#include <ray/util/copy_ptr.hpp>

namespace ray::compiler::S1::lang {

// early (Stage 1) symbol definition that contains the name and its type
struct S1Symbol {
	enum class SymbolType { Function, Struct, Variable, Parameter };
	SymbolType type;
	std::string name;
	// contains additional type info such as parameters or inner fields
	// for functions it is (returnType, ...paramType)
	std::vector<std::string> signature;
};

struct S1ScopeTable;

struct S1ScopeTable {
	std::optional<S1ScopeTable *> parentScope;
	std::string scopeName;
	std::vector<lang::S1Symbol> symbols;
	std::vector<util::copy_ptr<S1ScopeTable>> innerScopes;

	void clear() {
		symbols.clear();
		innerScopes.clear();
	}
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

class S1SourceUnit {
  public:
	std::vector<S1StructDeclaration> structDeclarations;
	std::vector<S1StructDefinition> structDefinitions;

	std::vector<S1FunctionDeclaration> functionDeclarations;

	S1ScopeTable rootScope;

	void clear();

	std::string exportSourceUnit() const;
	static S1SourceUnit importSourceUnit(std::stringstream &stream);

	// we do not need to export our symbol table for S1
	MSGPACK_DEFINE(structDeclarations, structDefinitions, functionDeclarations);
};

} // namespace ray::compiler::S1::lang
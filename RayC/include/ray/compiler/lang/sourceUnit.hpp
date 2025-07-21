#pragma once
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include <msgpack.hpp>

#include <ray/util/copy_ptr.hpp>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {
class Scope {
  public:
	std::unordered_map<std::string, lang::Type> variables;

	bool defineStruct(Type type);
};
class SourceUnit {
  public:
	std::vector<Scope> scopes = {Scope{}};
	// all functions and structs have a declaration vector
	// that comes from the previous top level resolver
	std::vector<StructDeclaration> structDeclarations;
	std::vector<FunctionDeclaration> functionDeclarations;

	std::vector<Struct> structDefinitions;
	std::vector<FunctionDefinition> functionDefinitions;

	std::optional<lang::Type> findStructType(const std::string &typeName) const;
};

// early (Stage 1) symbol definition that contains the name and its type
struct S1Symbol {
	enum class SymbolType { Function, Struct, Variable, Parameter };
	SymbolType type;
	std::string name;
	// contains additional type info such as parameters or inner fields
	// for functions it is (returnType, ...paramType)
	std::string signature;
};

struct S1SymbolTable;

struct S1SymbolTable {
	std::optional<S1SymbolTable *> parentScope;
	std::string scopeName;
	std::vector<lang::S1Symbol> symbols;
	std::vector<util::copy_ptr<S1SymbolTable>> innerScopes;

	void clear() {
		symbols.clear();
		innerScopes.clear();
	}
};

class S1SourceUnit {
  public:
	std::vector<S1StructDeclaration> structDeclarations;
	std::vector<S1StructDefinition> structDefinitions;

	std::vector<S1FunctionDeclaration> functionDeclarations;

	S1SymbolTable rootScope;

	void clear();

	std::string exportSourceUnit() const;
	static S1SourceUnit importSourceUnit(std::stringstream &stream);

	// we do not need to export our symbol table for S1
	MSGPACK_DEFINE(structDeclarations, structDefinitions, functionDeclarations);
};
} // namespace ray::compiler::lang
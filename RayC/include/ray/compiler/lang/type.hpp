#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <ray/util/copy_ptr.hpp>

namespace ray::compiler::lang {
class Type;
class Type {

	bool initialized = false;
	bool scalar = false;
	bool platformDependent = true;

  public:
	std::string name;
	size_t calculatedSize = 0;
	bool isMutable = false;
	bool isPointer = false;
	bool signedType = false;
	bool overloaded = false;
	std::optional<util::copy_ptr<Type>> subtype = std::nullopt;
	std::optional<std::vector<util::copy_ptr<Type>>> signature = std::nullopt;

	Type() = default;
	Type(bool initialized, bool scalar, bool platformDependent,
	     std::string name, size_t calculatedSize, bool isMutable,
	     bool isPointer, bool signedType, bool overloaded,
	     std::optional<util::copy_ptr<Type>> subType,
	     std::optional<std::vector<util::copy_ptr<Type>>> signature)
	    : initialized{initialized}, scalar{scalar},
	      platformDependent{platformDependent}, name{name},
	      calculatedSize{calculatedSize}, isMutable{isMutable},
	      isPointer{isPointer}, signedType{signedType}, overloaded{overloaded},
	      subtype{subType}, signature{signature} {};

	void initialize() { initialized = true; }
	bool isInitialized() const { return initialized; }
	bool isScalar() const { return scalar; }
	bool isPlatformDependent() const { return platformDependent; }

	bool coercercesInto(const Type &targetType) const;
	bool signatureEquals(const Type &targetType) const;
	bool signatureMatches(const Type &targetType) const;

	bool operator==(const Type &other) const;

	static std::optional<Type> findScalarType(const std::string_view name);

	// allows to define a struct type, if definition is external then the size
	// is 0 and can only be referenced as a pointer
	static Type defineStructType(std::string name, size_t aproximatedSize,
	                             bool platformDependent);
	// defines a new function type
	static Type defineFunctionType(Type returnType,
	                               std::vector<util::copy_ptr<Type>> signature);
	// defines a type that specifies that it is an overloaded function type
	static Type defineOverloadedFunctionType(Type returnType);
	// returns a type that is not instatiable and cannot be used
	// used by statements in the type checker
	static Type defineStmtType();

	// defines an empty module type used by the type checker
	static Type defineModuleType();

	// a shorcut for defineScalarType("void")
	static Type getVoidType();

	// converts a number expression into the smallest number type
	// available by the compiler
	static std::optional<Type>
	getNumberLiteralType(const std::string_view lexeme);

  private:
	bool baseMatches(const Type &other) const;

	static Type defineScalarType(std::string name, size_t calculatedSize,
	                             bool signedType);
	static Type definePlatformDependentType(std::string name,
	                                        size_t aproximatedSize,
	                                        bool signedType);
};
} // namespace ray::compiler::lang
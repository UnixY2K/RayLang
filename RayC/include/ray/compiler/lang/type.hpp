#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include <ray/util/copy_ptr.hpp>

namespace ray::compiler::lang {
class Type;
class Type {

	bool initialized = false;
	bool scalar = false;
	bool platformDependent = true;

  public:
	std::string name;
	std::string mangledName;
	size_t calculatedSize = 0;
	bool isConst = true;
	bool isPointer = false;
	bool signedType = false;
	std::optional<util::copy_ptr<Type>> subtype;

	Type() = default;
	Type(bool initialized, bool scalar, bool platformDependent,
	     std::string name, std::string mangledName, size_t calculatedSize,
	     bool isConst, bool isPointer, bool signedType,
	     std::optional<util::copy_ptr<Type>> subType)
	    : initialized{initialized}, scalar{scalar},
	      platformDependent{platformDependent}, name{name},
	      mangledName{mangledName}, calculatedSize{calculatedSize},
	      isConst{isConst}, isPointer{isPointer}, signedType{signedType},
	      subtype{subType} {};

	void initialize() { initialized = true; }
	bool isInitialized() const { return initialized; }
	bool isScalar() const { return scalar; }
	bool isPlatformDependent() const { return platformDependent; }

	static std::optional<Type> findScalarType(const std::string_view name);

	// allows to define a struct type, if definition is external then the size
	// is 0 and can only be referenced as a pointer
	static Type defineStructType(std::string name, size_t aproximatedSize,
	                             bool platformDependent);
	// defines a new function pointer type
	// TODO: have a lang::FunctionType that defines the required signature data
	// for the function so this data can be used by the compiler
	static Type defineFunctionType(std::string signature);

  private:
	static Type defineScalarType(std::string name, size_t calculatedSize,
	                             bool signedType);
	static Type definePlatformDependentType(std::string name,
	                                        size_t aproximatedSize,
	                                        bool signedType);
};
} // namespace ray::compiler::lang
#pragma once
#include <optional>

#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::environment {
class DataModel {
  public:
	// allows to define a struct type, if definition is external then the size
	// is 0 and can only be referenced as a pointer
	virtual lang::Type defineStructType(std::string name,
	                                    size_t aproximatedSize) const = 0;
	// defines a new function type
	virtual lang::Type defineFunctionType(
	    lang::Type returnType,
	    std::vector<util::copy_ptr<lang::Type>> signature) const = 0;
	// defines a type that specifies that it is an overloaded function type
	virtual lang::Type
	defineOverloadedFunctionType(lang::Type returnType) const = 0;

	// a shorcut for defineScalarType("void")
	virtual lang::Type getVoidType() const = 0;

	virtual std::optional<lang::Type>
	findScalarType(const std::string_view name) const = 0;

	virtual lang::Type definePointerType(lang::Type returnType) const = 0;

	// converts a number expression into the smallest number type
	// available by the compiler
	virtual std::optional<lang::Type>
	getNumberLiteralType(const std::string_view lexeme) const = 0;

	virtual ~DataModel() = default;
};
} // namespace ray::compiler::environment
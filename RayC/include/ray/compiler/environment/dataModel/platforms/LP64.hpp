#pragma once
#include <cstddef>
#include <ray/compiler/environment/dataModel/dataModel.hpp>

namespace ray::compiler::environment::dataModel::platforms {

class LP64 : public DataModel {
  public:
	// allows to define a struct type, if definition is external then the size
	// is 0 and can only be referenced as a pointer
	lang::Type defineStructType(std::string name,
	                            size_t aproximatedSize) const override;
	// defines a new function type
	lang::Type defineFunctionType(
	    lang::Type returnType,
	    std::vector<util::copy_ptr<lang::Type>> signature) const override;
	// defines a type that specifies that it is an overloaded function type
	lang::Type
	defineOverloadedFunctionType(lang::Type returnType) const override;

	// a shorcut for defineScalarType("void")
	lang::Type getVoidType() const override;

	std::optional<lang::Type>
	findScalarType(const std::string_view name) const override;

	lang::Type definePointerType(lang::Type returnType) const override;

	// converts a number expression into the smallest number type
	// available by the compiler
	std::optional<lang::Type>
	getNumberLiteralType(const std::string_view lexeme) const override;

	static LP64 &getInstance() {
		static LP64 instance;
		return instance;
	}

  private:
	static lang::Type defineScalarType(std::string name, size_t calculatedSize,
	                                   bool signedType);

	static constexpr size_t CHAR_SIZE = 1;      // 8
	static constexpr size_t SHORT_INT_SIZE = 2; // 16
	static constexpr size_t INT_SIZE = 4;       // 32
	static constexpr size_t LONG_INT_SIZE = 8;  // 64
	static constexpr size_t LONG_LONG_SIZE = 8; // 64
	static constexpr size_t POINTER_SIZE = 8;   // 64
};

} // namespace ray::compiler::environment::dataModel::platforms
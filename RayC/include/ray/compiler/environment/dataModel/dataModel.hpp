#pragma once
#include <cstddef>
#include <optional>

#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::environment {
class DataModel {
  public:
	// allows to define a struct type, if definition is external then the size
	// is 0 and can only be referenced as a pointer
	lang::Type defineStructType(std::string name, size_t aproximatedSize) const;
	// declares an struct type whose internal details are unknown
	lang::Type declareStructType(std::string name) const;
	// defines a new function type
	lang::Type
	defineFunctionType(lang::Type returnType,
	                   std::vector<util::copy_ptr<lang::Type>> signature) const;
	// defines a type that specifies that it is an overloaded function type
	lang::Type defineOverloadedFunctionType(lang::Type returnType) const;

	// a shorcut for defineScalarType("void")
	lang::Type getVoidType() const;

	std::optional<lang::Type> findScalarType(const std::string_view name) const;

	lang::Type defineScalarType(std::string name, size_t calculatedSize,
	                            bool signedType) const;

	lang::Type definePointerType(lang::Type returnType) const;

	// converts a number expression into the smallest number type
	// available by the compiler
	std::optional<lang::Type>
	getNumberLiteralType(const std::string_view lexeme) const;

	const size_t charSize;
	const size_t shortIntSize;
	const size_t intSize;
	const size_t longIntSize;
	const size_t longLongSize;
	const size_t pointerSize;

	DataModel(const size_t charSize, const size_t shortIntSize,
	          const size_t intSize, const size_t longIntSize,
	          const size_t longLongSize, const size_t pointerSize)
	    : charSize(charSize), shortIntSize(shortIntSize), intSize(intSize),
	      longIntSize(longIntSize), longLongSize(longLongSize),
	      pointerSize(pointerSize) {}

	static DataModel &LLP64DataModel() {
		static constexpr size_t CHAR_SIZE = 1;      // 8
		static constexpr size_t SHORT_INT_SIZE = 2; // 16
		static constexpr size_t INT_SIZE = 4;       // 32
		static constexpr size_t LONG_INT_SIZE = 4;  // 32
		static constexpr size_t LONG_LONG_SIZE = 8; // 64
		static constexpr size_t POINTER_SIZE = 8;   // 64
		static DataModel dataModel(CHAR_SIZE, SHORT_INT_SIZE, INT_SIZE,
		                           LONG_INT_SIZE, LONG_LONG_SIZE, POINTER_SIZE);
		return dataModel;
	}

	static DataModel &LP64DataModel() {
		static constexpr size_t CHAR_SIZE = 1;      // 8
		static constexpr size_t SHORT_INT_SIZE = 2; // 16
		static constexpr size_t INT_SIZE = 4;       // 32
		static constexpr size_t LONG_INT_SIZE = 8;  // 64
		static constexpr size_t LONG_LONG_SIZE = 8; // 64
		static constexpr size_t POINTER_SIZE = 8;   // 64
		static DataModel dataModel(CHAR_SIZE, SHORT_INT_SIZE, INT_SIZE,
		                           LONG_INT_SIZE, LONG_LONG_SIZE, POINTER_SIZE);
		return dataModel;
	}
};
} // namespace ray::compiler::environment
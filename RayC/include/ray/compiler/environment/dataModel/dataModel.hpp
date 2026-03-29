#pragma once
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

#include <ray/compiler/lang/type.hpp>
#include <ray/util/copy_ptr.hpp>

namespace ray::compiler::environment {
class DataModel {
  public:
	enum class ScalarTypeKind {
		boolScalar,
		u8Scalar,
		s8Scalar,
		u16Scalar,
		s16Scalar,
		u32Scalar,
		s32Scalar,
		u64Scalar,
		s64Scalar,
		f32Scalar,
		f64Scalar,
		usizeScalar,
		ssizeScalar,
		c_charScalar,
		c_intScalar,
		c_sizeScalar,
		// undefined type used to keep track of max lenght of enum
		ScalarTypeKindUndefined,
	};

	static constexpr const std::optional<const std::string_view>
	getScalarTypeKindString(const ScalarTypeKind kind) {
		static constexpr std::array<
		    std::string_view,
		    static_cast<size_t>(
		        DataModel::ScalarTypeKind::ScalarTypeKindUndefined)>
		    values = {

		    };
		return values.at(static_cast<size_t>(kind));
	}
	static const std::optional<const ScalarTypeKind>
	toScalarTypeKind(const std::string_view name);

	// allows to define a tuple type, its signature holds
	lang::Type
	defineTupleType(size_t tupleID,
	                std::vector<util::copy_ptr<lang::Type>> signature,
	                size_t aproximatedSize) const;
	// allows to define a struct type, if definition is external/unknown then
	// the size is 0 and can only be referenced as a pointer
	lang::Type defineStructType(size_t structID, std::string name,
	                            size_t aproximatedSize) const;
	lang::Type defineTraitType(
	    size_t traitID, std::string name,
	    std::vector<util::copy_ptr<lang::Type>> methodSignature) const;
	// defines a new function type
	lang::Type
	defineFunctionType(lang::Type returnType,
	                   std::vector<util::copy_ptr<lang::Type>> signature) const;
	// defines a type that specifies that it is an overloaded function type
	lang::Type defineOverloadedFunctionType(lang::Type returnType) const;

	lang::Type
	defineMethodType(lang::Type returnType,
	                 std::vector<util::copy_ptr<lang::Type>> signature) const;
	lang::Type defineOverloadedMethodType(lang::Type returnType) const;

	// defines an abstract empty tuple
	lang::Type getUnitType(bool isMutable = false) const;

	lang::Type getScalarType(const ScalarTypeKind scalarKind) const;
	std::optional<lang::Type> findScalarType(const std::string_view name) const;

	static lang::Type defineScalarType(std::string name, size_t calculatedSize,
	                                   bool signedType, bool isMutable);

	lang::Type definePointerType(lang::Type returnType, bool isMutable) const;

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
#include <charconv>
#include <cstddef>
#include <format>
#include <optional>
#include <string_view>
#include <system_error>
#include <unordered_map>

#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::environment {

const std::optional<const DataModel::ScalarTypeKind>
DataModel::toScalarTypeKind(const std::string_view name) {
	static std::unordered_map<std::string, ScalarTypeKind> map = {
	    {
	        "bool",
	        DataModel::ScalarTypeKind::boolScalar,
	    }, // bool
	    {
	        "u8",
	        DataModel::ScalarTypeKind::u8Scalar,
	    }, // u8
	    {
	        "s8",
	        DataModel::ScalarTypeKind::s8Scalar,
	    }, // s8
	    {
	        "u16",
	        DataModel::ScalarTypeKind::u16Scalar,
	    }, // u16
	    {
	        "s16",
	        DataModel::ScalarTypeKind::s16Scalar,
	    }, // s16
	    {
	        "u32",
	        DataModel::ScalarTypeKind::u32Scalar,
	    }, // u32
	    {
	        "s32",
	        DataModel::ScalarTypeKind::s32Scalar,
	    }, // s32
	    {
	        "u64",
	        DataModel::ScalarTypeKind::u64Scalar,
	    }, // u64
	    {
	        "s64",
	        DataModel::ScalarTypeKind::s64Scalar,
	    }, // s64
	    {
	        "f32",
	        DataModel::ScalarTypeKind::f32Scalar,
	    }, // f32
	    {
	        "f64",
	        DataModel::ScalarTypeKind::f64Scalar,
	    }, // f64
	    {
	        "usize",
	        DataModel::ScalarTypeKind::usizeScalar,
	    }, // usize
	    {
	        "ssize",
	        DataModel::ScalarTypeKind::ssizeScalar,
	    }, // ssize
	    {
	        "c_char",
	        DataModel::ScalarTypeKind::c_charScalar,
	    }, // c_char
	    {
	        "c_int",
	        DataModel::ScalarTypeKind::c_intScalar,
	    }, // c_int
	    {
	        "c_size",
	        DataModel::ScalarTypeKind::c_sizeScalar,
	    }, // c_size
	};
	std::string key{name};
	return map.contains(key) ? std::optional<ScalarTypeKind>(map.at(key))
	                         : std::nullopt;
}

lang::Type
DataModel::defineTupleType(size_t tupleID,
                           std::vector<util::copy_ptr<lang::Type>> signature,
                           size_t aproximatedSize) const {
	return {
	    // its type is the same as structID
	    tupleID,
	    // initialized
	    true,
	    lang::TypeKind::aggregate,
	    "%<tuple>%",
	    aproximatedSize,
	    false,     // non mutable
	    false,     // non signed
	    false,     // cannot be overloaded
	    {},        // no subtype
	    signature, // no signature? depending on layout and underlying
	               // architecture cannot be guaranteed
	};
}

lang::Type DataModel::defineStructType(size_t structID, std::string name,
                                       size_t aproximatedSize) const {
	return {
	    // its type is the same as structID
	    structID,
	    // initialized
	    true,
	    lang::TypeKind::aggregate,
	    name,
	    aproximatedSize,
	    false, // non mutable
	    false, // non signed
	    false, // cannot be overloaded
	    {},    // no subtype
	    {},    // no signature? depending on layout and underlying architecture
	           // cannot be guaranteed
	};
}

lang::Type DataModel::defineFunctionType(
    lang::Type returnType,
    std::vector<util::copy_ptr<lang::Type>> signature) const {
	auto fn = definePointerType(returnType, false);
	// TODO: remove this ugly hack once the new type scanner is set in place
	fn.name = "//fn";
	fn.signature = signature;
	return fn;
}

lang::Type
DataModel::defineOverloadedFunctionType(lang::Type returnType) const {
	return lang::Type(
	    // no type ID
	    0,
	    // initialized as its overloads are initialized
	    true,
	    // a pointer is not a scalar as it is an address memory
	    // that references an object
	    lang::TypeKind::pointer,
	    // define the name as pointer
	    "//fn-overload",
	    0,          // an overloaded function does not hold any size
	    false,      // if the pointer type is const or not is decided later
	    false,      // non signed, is a pointer
	    true,       // we hold a function pointer so it is not overloaded
	    returnType, // same as a function type
	    {}          // no signature data as the parent expression has to
	                // resolve the desired overload
	);
}

lang::Type DataModel::getUnitType(bool isMutable) const {
	return lang::Type::defineUnitType(isMutable);
}

lang::Type DataModel::getScalarType(const ScalarTypeKind scalarKind) const {
	static std::unordered_map<ScalarTypeKind, lang::Type> map = {
	    {
	        DataModel::ScalarTypeKind::boolScalar,
	        defineScalarType("bool", 1, false, false),
	    }, // bool
	    {
	        DataModel::ScalarTypeKind::u8Scalar,
	        defineScalarType("u8", 1, false, false),
	    }, // u8
	    {
	        DataModel::ScalarTypeKind::s8Scalar,
	        defineScalarType("s8", 1, true, false),
	    }, // s8
	    {
	        DataModel::ScalarTypeKind::u16Scalar,
	        defineScalarType("u16", 2, false, false),
	    }, // u16
	    {
	        DataModel::ScalarTypeKind::s16Scalar,
	        defineScalarType("s16", 2, true, false),
	    }, // s16
	    {
	        DataModel::ScalarTypeKind::u32Scalar,
	        defineScalarType("u32", 4, false, false),
	    }, // u32
	    {
	        DataModel::ScalarTypeKind::s32Scalar,
	        defineScalarType("s32", 4, true, false),
	    }, // s32
	    {
	        DataModel::ScalarTypeKind::u64Scalar,
	        defineScalarType("u64", 8, false, false),
	    }, // u64
	    {
	        DataModel::ScalarTypeKind::s64Scalar,
	        defineScalarType("s64", 8, true, false),
	    }, // s64
	    {
	        DataModel::ScalarTypeKind::f32Scalar,
	        defineScalarType("f32", 4, true, false),
	    }, // f32
	    {
	        DataModel::ScalarTypeKind::f64Scalar,
	        defineScalarType("f64", 8, true, false),
	    }, // f64
	    {
	        DataModel::ScalarTypeKind::usizeScalar,
	        defineScalarType("usize", pointerSize, false, false),
	    }, // usize
	    {
	        DataModel::ScalarTypeKind::ssizeScalar,
	        defineScalarType("ssize", pointerSize, true, false),
	    }, // ssize
	    {
	        DataModel::ScalarTypeKind::c_charScalar,
	        defineScalarType("c_char", charSize, true, false),
	    }, // c_char
	    {
	        DataModel::ScalarTypeKind::c_intScalar,
	        defineScalarType("c_int", intSize, true, false),
	    }, // c_int
	    {
	        DataModel::ScalarTypeKind::c_sizeScalar,
	        defineScalarType("c_size", pointerSize, false, false),
	    }, // c_size
	};
	return map.at(scalarKind);
}

std::optional<lang::Type>
DataModel::findScalarType(const std::string_view name) const {
	return toScalarTypeKind(name).transform(
	    [this](const DataModel::ScalarTypeKind kind) -> lang::Type {
		    return this->getScalarType(kind);
	    });
}

lang::Type DataModel::defineScalarType(std::string name, size_t calculatedSize,
                                       bool signedType, bool isMutable) {
	return lang::Type{
	    // scalars do not have typeID
	    0,
	    true,                   // initialized type
	    lang::TypeKind::scalar, // it is an scalar type
	    name,                   // specified name
	    calculatedSize, // size is known even without platform information
	    isMutable,      // by default all scalar types are const
	    signedType,     //	only signed if specified
	    false,          // no overload
	    std::nullopt,   // no subtype
	    std::nullopt,   // no signature
	};
}

lang::Type DataModel::definePointerType(lang::Type returnType,
                                        bool isMutable) const {
	return lang::Type{
	    // pointers do not have typeID
	    0,
	    true,                    // initialized type
	    lang::TypeKind::pointer, // it is an scalar type
	    "%<pointer>%",           // specified name
	    pointerSize,             // varies depending on the data model
	    isMutable,               // by default all scalar types are const
	    true,                    // is not a pointer type
	    false,                   // no overload
	    returnType,              // its subtype is the returnType
	    std::nullopt,            // no signature
	};
}

// converts a number expression into the smallest number type
// available by the compiler
std::optional<lang::Type>
DataModel::getNumberLiteralType(const std::string_view lexeme) const {
	// for now assume that is the largest type
	// at a later step we will worry about smaller types like f32 or the integer
	// sides
	bool floatingPoint = false;
	bool signedNumber = false;
	size_t minimalBits = 32;
	size_t pos = 0;
	if (lexeme.empty()) {
		return std::nullopt;
	}

	const char sign = lexeme[0];
	if (sign == '+' || sign == '-') {
		signedNumber = sign == '-';
		pos++;
	}

	size_t numEnd = 0;
	for (; pos < lexeme.size(); pos++) {
		const char digit = lexeme[pos];
		const int dVal = digit - '0';
		if (dVal >= 0 && dVal <= 9) {
			numEnd = pos;
			continue;
		}
		if (digit == '.') {
			floatingPoint = true;
			break;
		}
		break;
	}

	if (numEnd + 1 < lexeme.size()) {
		return findScalarType(std::string_view(lexeme).substr(numEnd));
	}

	const char *parse_start_ptr = lexeme.data();
	const char *parse_end_ptr = lexeme.data() + numEnd + 1;
	if (floatingPoint) {
		minimalBits = 32;
		double sParsedVal;
		auto [ptr, ec] =
		    std::from_chars(parse_start_ptr, parse_end_ptr, sParsedVal);
		if (ec == std::errc::invalid_argument ||
		    ec == std::errc::result_out_of_range) {
			return std::nullopt;
		}

		// if our number is greater than the limit of 32 bits upgrade to 64
		if (sParsedVal < std::numeric_limits<float>::min()) {
			minimalBits = 64;
		}
	} else {
		if (signedNumber) {
			int64_t sParsedVal;
			auto [ptr, ec] =
			    std::from_chars(parse_start_ptr, parse_end_ptr, sParsedVal);
			if (ec == std::errc::invalid_argument ||
			    ec == std::errc::result_out_of_range) {
				return std::nullopt;
			}

			// if our number is greater than the limit of 32 bits upgrade to 64
			if (sParsedVal < std::numeric_limits<int32_t>::min()) {
				minimalBits = 64;
			}
		} else {
			uint64_t uParsedVal;
			auto [ptr, ec] =
			    std::from_chars(parse_start_ptr, parse_end_ptr, uParsedVal);
			if (ec == std::errc::invalid_argument ||
			    ec == std::errc::result_out_of_range) {
				return std::nullopt;
			}
			// check the smallest type of (s32->u32(default size)->s64->u64)
			if (uParsedVal <=
			    static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
				signedNumber = true;
			} else if (uParsedVal <= static_cast<uint64_t>(
			                             std::numeric_limits<int64_t>::max())) {
				signedNumber = true;
				minimalBits = 64;
			} else if (uParsedVal > static_cast<uint64_t>(
			                            std::numeric_limits<uint32_t>::max())) {
				minimalBits = 64;
			}
		}
	}
	std::string typeLexeme = std::format(
	    "{}{}",
	    floatingPoint ? "f"
	    : signedNumber
	        ? "s" // This logic will be updated based on actual parsed value
	        : "u",
	    minimalBits);

	return findScalarType(typeLexeme);
}
} // namespace ray::compiler::environment
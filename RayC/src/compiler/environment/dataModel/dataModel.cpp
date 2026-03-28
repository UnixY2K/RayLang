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

std::optional<lang::Type>
DataModel::findScalarType(const std::string_view name) const {
	static std::unordered_map<std::string, lang::Type> map = {
	    {
	        "bool",
	        defineScalarType("bool", 1, false, false),
	    }, // bool
	    {
	        "u8",
	        defineScalarType("u8", 1, false, false),
	    }, // u8
	    {
	        "s8",
	        defineScalarType("s8", 1, true, false),
	    }, // s8
	    {
	        "u16",
	        defineScalarType("u16", 2, false, false),
	    }, // u16
	    {
	        "s16",
	        defineScalarType("s16", 2, true, false),
	    }, // s16
	    {
	        "u32",
	        defineScalarType("u32", 4, false, false),
	    }, // u32
	    {
	        "s32",
	        defineScalarType("s32", 4, true, false),
	    }, // s32
	    {
	        "u64",
	        defineScalarType("u64", 8, false, false),
	    }, // u64
	    {
	        "s64",
	        defineScalarType("s64", 8, true, false),
	    }, // s64
	    {
	        "f32",
	        defineScalarType("f32", 4, true, false),
	    }, // f32
	    {
	        "f64",
	        defineScalarType("f64", 8, true, false),
	    }, // f64
	    {
	        "usize",
	        defineScalarType("usize", pointerSize, false, false),
	    }, // usize
	    {
	        "ssize",
	        defineScalarType("ssize", pointerSize, true, false),
	    }, // ssize
	    {
	        "c_char",
	        defineScalarType("c_char", charSize, true, false),
	    }, // c_char
	    {
	        "c_int",
	        defineScalarType("c_int", intSize, true, false),
	    }, // c_int
	    {
	        "c_size",
	        defineScalarType("c_size", pointerSize, false, false),
	    }, // c_size
	};
	std::string key{name};
	return map.contains(key) ? std::optional<lang::Type>(map.at(key))
	                         : std::nullopt;
}

lang::Type DataModel::defineScalarType(std::string name, size_t calculatedSize,
                                       bool signedType, bool isMutable) const {
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
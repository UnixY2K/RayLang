#include <charconv>
#include <cstddef>
#include <format>
#include <optional>
#include <string_view>
#include <unordered_map>

#include <ray/compiler/environment/dataModel/dataModel.hpp>

namespace ray::compiler::environment {

lang::Type DataModel::defineStructType(std::string name,
                                       size_t aproximatedSize) const {
	return {
	    // by default its type is unknown
	    0,
	    // initialized
	    true,
	    // non-scalar
	    false,
	    name,
	    aproximatedSize,
	    false, // non mutable
	    false, // non pointer
	    false, // non signed
	    false, // cannot be overloaded
	    {},    // no subtype
	    {},    // no signature? depending on layout and underlying architecture
	           // cannot be guaranteed
	};
}

lang::Type DataModel::declareStructType(std::string name) const {
	return {
	    // by default its typeID is unknown
	    0,
	    // initialized
	    true,
	    // non-scalar
	    false,
	    name,
	    // unknown size
	    0,
	    false, // non mutable
	    false, // non pointer
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
	return definePointerType(lang::Type(
	    // pointer do not have a known typeID
	    0,
	    // known initializable value
	    true,
	    // a pointer is not a scalar as it is an address memory
	    // that references an object
	    false,
	    "//fn", // define the name as pointer
	    0, // a function is itself a pointer, so no size of its underlying type
	    false,      // if the pointer type is const or not is decided later
	    false,      // a funciton is a just an memory address(pointer)
	    false,      // non signed, is a pointer
	    false,      // we hold a function pointer so it is not overloaded
	    returnType, // contains the return value of the caller
	    signature // signature data is always provided so the caller can know it
	              // is valid to call it
	    ));
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
	    false,
	    // define the name as pointer
	    "//fn-overload",
	    0,          // an overloaded function does not hold any size
	    false,      // if the pointer type is const or not is decided later
	    true,       // a funciton is a just an memory address(pointer)
	    false,      // non signed, is a pointer
	    true,       // we hold a function pointer so it is not overloaded
	    returnType, // same as a function type
	    {}          // no signature data as the parent expression has to
	                // resolve the desired overload
	);
}

lang::Type DataModel::getVoidType() const {
	return defineScalarType("void", 0, false);
}

std::optional<lang::Type>
DataModel::findScalarType(const std::string_view name) const {
	static std::unordered_map<std::string, lang::Type> map = {
	    {
	        "bool",
	        defineScalarType("bool", 1, false),
	    }, // bool
	    {
	        "u8",
	        defineScalarType("u8", 1, false),
	    }, // u8
	    {
	        "s8",
	        defineScalarType("s8", 1, true),
	    }, // s8
	    {
	        "u16",
	        defineScalarType("u16", 2, false),
	    }, // u16
	    {
	        "s16",
	        defineScalarType("s16", 2, true),
	    }, // s16
	    {
	        "u32",
	        defineScalarType("u32", 4, false),
	    }, // u32
	    {
	        "s32",
	        defineScalarType("s32", 4, true),
	    }, // s32
	    {
	        "u64",
	        defineScalarType("u64", 8, false),
	    }, // u64
	    {
	        "s64",
	        defineScalarType("s64", 8, true),
	    }, // s64
	    {
	        "f32",
	        defineScalarType("f32", 4, true),
	    }, // f32
	    {
	        "f64",
	        defineScalarType("f64", 8, true),
	    }, // f64
	    {
	        "usize",
	        defineScalarType("usize", pointerSize, false),
	    }, // usize
	    {
	        "ssize",
	        defineScalarType("ssize", pointerSize, true),
	    }, // ssize
	    {
	        "c_char",
	        defineScalarType("c_char", charSize, true),
	    }, // c_char
	    {
	        "c_int",
	        defineScalarType("c_int", intSize, true),
	    }, // c_int
	    {
	        "c_size",
	        defineScalarType("c_size", pointerSize, false),
	    }, // c_size
	    {
	        "void",
	        getVoidType(),
	    }, // unit type
	};
	std::string key{name};
	return map.contains(key) ? std::optional<lang::Type>(map.at(key))
	                         : std::nullopt;
}

lang::Type DataModel::defineScalarType(std::string name, size_t calculatedSize,
                                       bool signedType) const {
	return lang::Type{
	    // scalars do not have typeID
	    0,
	    true,           // initialized type
	    true,           // it is an scalar type
	    name,           // specified name
	    calculatedSize, // size is known even without platform information
	    false,          // by default all scalar types are const
	    false,          // is not a pointer type
	    signedType,     //	only signed if specified
	    false,          // no overload
	    std::nullopt,   // no subtype
	    std::nullopt,   // no signature
	};
}

lang::Type DataModel::definePointerType(lang::Type returnType) const {
	return lang::Type{
	    // pointers do not have typeID
	    0,
	    true,          // initialized type
	    true,          // it is an scalar type
	    "%<pointer>%", // specified name
	    pointerSize,   // varies depending on the data model
	    false,         // by default all scalar types are const
	    true,          // is not a pointer type
	    false,         // a pointer is not signed
	    false,         // no overload
	    returnType,    // its subtype is the returnType
	    std::nullopt,  // no signature
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
			continue;
		}
		if (digit == '.') {
			floatingPoint = true;
			break;
		}
		// break anyways and set this as postfix
		numEnd = pos;
		break;
	}

	if (numEnd + 1 < lexeme.size()) {
		return findScalarType(std::string_view(lexeme).substr(numEnd));
	}

	if (floatingPoint) {
		// TODO: add floating point parse code
		minimalBits = 64;
	} else {
		const char *parse_start_ptr = lexeme.data();
		const char *parse_end_ptr = lexeme.data() + numEnd + 1;
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
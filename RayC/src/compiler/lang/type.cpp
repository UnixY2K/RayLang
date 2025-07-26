#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <optional>
#include <unordered_map>

#include <ray/compiler/lang/type.hpp>
namespace ray::compiler::lang {

bool Type::operator==(const Type &other) const {
	if (initialized == other.initialized &&                  //
	    scalar == other.scalar &&                            //
	    platformDependent == other.platformDependent &&      //
	    name == other.name &&                                //
	    mangledName == other.mangledName &&                  //
	    calculatedSize == other.calculatedSize &&            //
	    isMutable == other.isMutable &&                      //
	    isPointer == other.isPointer &&                      //
	    signedType == other.signedType &&                    //
	    subtype.has_value() == other.subtype.has_value() &&  //
	    signature.has_value() == other.signature.has_value() //
	) {
		if (subtype.has_value() && *subtype.value() != *other.subtype.value()) {
			return false;
		}
		if (signature.has_value()) {
			const auto &lhsSignature = signature.value();
			const auto &rhsSignature = other.signature.value();

			if (lhsSignature.size() != rhsSignature.size()) {
				return false;
			}
			for (size_t i = 0; i < lhsSignature.size(); i++) {
				if (*lhsSignature[i] != *rhsSignature[i]) {
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

std::optional<Type> Type::findScalarType(const std::string_view name) {
	static std::unordered_map<std::string, Type> map = {
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
	        definePlatformDependentType("usize", 8, false),
	    }, // usize
	    {
	        "ssize",
	        definePlatformDependentType("ssize", 8, true),
	    }, // ssize
	    {
	        "c_char",
	        definePlatformDependentType("c_char", 1, true),
	    }, // c_char
	    {
	        "c_int",
	        definePlatformDependentType("c_int", 4, true),
	    }, // c_int
	    {
	        "c_size",
	        definePlatformDependentType("c_size", 8, false),
	    }, // c_size
	    {
	        "c_voidptr",
	        Type{
	            true,
	            true,
	            true, // the size is determined by platform
	            "c_voidptr",
	            "c_voidptr",
	            8, // estimated size for 64bits arch
	            false,
	            false, // set as false as cast is "unsafe"
	            false,
	            false, // non overloaded
	            {},    // no subtype
	            {},    // no signature
	        },
	    }, // c_voidptr
	    {
	        "()",
	        defineScalarType("()", 0, false),
	    }, // unit type
	};
	std::string key{name};
	return map.contains(key) ? std::optional<Type>(map.at(key)) : std::nullopt;
}

Type Type::defineStructType(std::string name, size_t aproximatedSize,
                            bool platformDependent) {
	return Type{
	    true,
	    false,
	    // is platform dependent only if one of the struct members
	    // is platform dependent, we need to fix this in the future
	    // by having the compiler define the platform depentent scalars
	    // such as the C types, or make them conditionally defined types at
	    // compile time
	    platformDependent,
	    name,
	    name, //
	    aproximatedSize,
	    false,
	    false,
	    false,
	    false,
	    // we do not have subtypes for structs as we are not structs
	    {}, //
	    {}, //
	};
}

Type Type::defineFunctionType(Type returnType,
                              std::vector<util::copy_ptr<Type>> signature) {
	return lang::Type(
	    true,
	    // a pointer is not a scalar as it is an address memory
	    // that references an object
	    false,
	    // technically platform dependent on pointer definition
	    true,
	    "//fn", // define the name as pointer
	    "//fn", // TODO: make a symbol mangler separate to type info
	    // we need to get this from the platform in the future
	    // for now assuming 64bits/8bytes
	    8,          // TODO: make this be obtained from platform configuration
	    false,      // if the pointer type is const or not is decided later
	    true,       // a funciton is a just an memory address(pointer)
	    false,      // non signed, is a pointer
	    false,      // we hold a function pointer so it is not overloaded
	    returnType, // contains the return value of the caller
	    signature // signature data is always provided so the caller can know it
	              // is valid to call it
	);
}

Type Type::defineOverloadedFunctionType(Type returnType) {
	return lang::Type(
	    true,
	    // a pointer is not a scalar as it is an address memory
	    // that references an object
	    false,
	    // technically platform dependent on pointer definition
	    true,
	    // define the name as pointer
	    "//fn-overload",
	    "//fn-overload", // TODO: make a symbol mangler
	    // we need to get this from the platform in the future
	    // for now assuming 64bits/8bytes
	    8,          // TODO: make this be obtained from platform configuration
	    false,      // if the pointer type is const or not is decided later
	    true,       // a funciton is a just an memory address(pointer)
	    false,      // non signed, is a pointer
	    true,       // we hold a function pointer so it is not overloaded
	    returnType, // same as a function type
	    {}          // no signature data as the parent expression has to
	                // resolve the desired overload
	);
}

Type Type::defineStmtType() {
	return Type{
	    // an statement does not even return an initialized type
	    false,
	    true,
	    false,
	    // name cannot be mangled and referenced
	    "%<stmt>%",
	    "%<stmt>%",
	    // size is 0 so it cannot be passed
	    0,
	    false,
	    false,
	    false,
	    false,
	    {}, //
	    {}, //
	};
}

std::optional<Type> Type::getNumberLiteralType(const std::string_view lexeme) {
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

	for (; pos < lexeme.size(); pos++) {
		const char digit = lexeme[pos];
		if (digit == '.') {
			floatingPoint = true;
			break;
		}
	}

	const char *parse_start_ptr = lexeme.data();
	const char *parse_end_ptr = lexeme.data() + lexeme.size();

	if (floatingPoint) {
		// TODO
		minimalBits = 64;
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

Type Type::defineScalarType(std::string name, size_t calculatedSize,
                            bool signedType) {
	return Type{
	    true,           // initialized type
	    true,           // it is an scalar type
	    false,          // is not platform dependent
	    name,           // specified name
	    name,           //	 scalar types do not have mangled type
	    calculatedSize, // size is known even without platform information
	    false,          // by default all scalar types are const
	    false,          // is not a pointer type
	    signedType,     //	only signed if specified
	    false,          // no overload
	    std::nullopt,   // no subtype
	    std::nullopt,   // no signature
	};
}
Type Type::definePlatformDependentType(std::string name, size_t aproximatedSize,
                                       bool signedType) {
	return Type{
	    true, // known initialized type
	    true, // assume that is scalar
	    true, //  platform dependent
	    name,
	    name, //
	    aproximatedSize,
	    false, // const unless told is not
	    false, // non pointer
	    signedType,
	    false, // non overloaded type
	    {},    // no subtype
	    {}     // no signature
	};
}

} // namespace ray::compiler::lang
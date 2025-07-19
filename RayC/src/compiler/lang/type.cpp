#include <format>
#include <optional>
#include <unordered_map>

#include <ray/compiler/lang/type.hpp>
namespace ray::compiler::lang {

bool Type::operator==(const Type &other) const {
	if (initialized == other.initialized &&             //
	    scalar == other.scalar &&                       //
	    platformDependent == other.platformDependent && //
	    name == other.name &&                           //
	    mangledName == other.mangledName &&             //
	    calculatedSize == other.calculatedSize &&       //
	    isConst == other.isConst &&                     //
	    isPointer == other.isPointer &&                 //
	    signedType == other.signedType &&               //
	    subtype.has_value() == other.subtype.has_value()) {
		return subtype.has_value() ? *subtype.value() == *other.subtype.value()
		                           : true;
	}
	return false;
}

std::optional<Type> Type::findScalarType(const std::string_view name) {
	static std::unordered_map<std::string, Type> map = {
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
	            {} // no subtype
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
	    // we do not have subtypes for structs as we are not structs
	    {}, //
	};
}

Type Type::defineFunctionType(std::string signature) {
	std::string pointerName = std::format("fn({})", signature);
	return lang::Type(
	    true,
	    // a pointer is not a scalar as it is an address memory
	    // that references an object
	    false,
	    // technically platform dependent on pointer definition
	    true,
	    // define the name as pointer
	    pointerName,
	    // for now lets just copy the type name, even if is not valid
	    pointerName,
	    // we need to get this from the platform in the future
	    // for now assuming 64bits/8bytes
	    8,
	    // if the pointer type is const or not is decided later
	    false,
	    // we are a pointer type
	    true,
	    // a pointer is not a signed type
	    false,
	    // our inner pointer type
	    {});
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
	    {}, //
	};
}

std::optional<Type> Type::getNumberLiteralType(const std::string_view lexeme) {
	// stub method
	return {};
}

Type Type::defineScalarType(std::string name, size_t calculatedSize,
                            bool signedType) {
	return Type{
	    true,           //
	    true,           //
	    false,          //
	    name,           //
	    name,           //
	    calculatedSize, //
	    false,
	    false,      //
	    signedType, //
	    {},         //
	};
}
Type Type::definePlatformDependentType(std::string name, size_t aproximatedSize,
                                       bool signedType) {
	return Type{
	    true,
	    true,
	    false,
	    name,
	    name, //
	    aproximatedSize,
	    false,
	    false,
	    signedType,
	    {}, //
	};
}

} // namespace ray::compiler::lang
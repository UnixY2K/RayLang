#include <optional>
#include <unordered_map>

#include <ray/compiler/lang/type.hpp>
namespace ray::compiler::lang {

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
	};
	std::string key{name};
	return map.contains(key) ? std::optional<Type>(map.at(key)) : std::nullopt;
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
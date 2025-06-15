#include <optional>
#include <unordered_map>

#include <ray/compiler/types/types.hpp>

namespace ray::compiler::types {
std::optional<TypeInfo>
TypeInfo::findScalarTypeInfo(const std::string_view lexeme) {
	static std::unordered_map<std::string, TypeInfo> map = {
	    {
	        "u8",
	        TypeInfo{
	            .name = "u8",
	            .mangledName = "u8",
	            .calculatedSize = 1,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // u8
	    {
	        "s8",
	        TypeInfo{
	            .name = "s8",
	            .mangledName = "s8",
	            .calculatedSize = 1,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // s8
	    {
	        "u16",
	        TypeInfo{
	            .name = "u16",
	            .mangledName = "u16",
	            .calculatedSize = 2,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // u16
	    {
	        "s16",
	        TypeInfo{
	            .name = "s16",
	            .mangledName = "s16",
	            .calculatedSize = 2,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // s16
	    {
	        "u32",
	        TypeInfo{
	            .name = "u32",
	            .mangledName = "u32",
	            .calculatedSize = 4,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // u32
	    {
	        "s32",
	        TypeInfo{
	            .name = "s32",
	            .mangledName = "s32",
	            .calculatedSize = 4,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // s32
	    {
	        "u64",
	        TypeInfo{
	            .name = "u64",
	            .mangledName = "u64",
	            .calculatedSize = 8,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // u64
	    {
	        "s64",
	        TypeInfo{
	            .name = "s64",
	            .mangledName = "s64",
	            .calculatedSize = 8,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // s64
	    {
	        "f32",
	        TypeInfo{
	            .name = "f32",
	            .mangledName = "f32",
	            .calculatedSize = 4,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // f32
	    {
	        "f64",
	        TypeInfo{
	            .name = "f64",
	            .mangledName = "f64",
	            .calculatedSize = 8,
	            .isScalar = true,
	            .platformDependent = false,
	        },
	    }, // f64
	    {
	        "usize",
	        TypeInfo{
	            .name = "usiz",
	            .mangledName = "usize",
	            .calculatedSize = 8,
	            .isScalar = true,
	            .platformDependent = true,
	        },
	    }, // usize
	    {
	        "ssize",
	        TypeInfo{
	            .name = "ssize",
	            .mangledName = "ssize",
	            .calculatedSize = 8,
	            .isScalar = true,
	            .platformDependent = true,
	        },
	    }, // ssize
	    {
	        "c_char",
	        TypeInfo{
	            .name = "c_char",
	            .mangledName = "c_char",
	            .calculatedSize = 1,
	            .isScalar = true,
	            .platformDependent = true,
	        },
	    }, // c_char
	    {
	        "c_int",
	        TypeInfo{
	            .name = "c_int",
	            .mangledName = "c_int",
	            .calculatedSize = 4,
	            .isScalar = true,
	            .platformDependent = true,
	        },
	    }, // c_int
	    {
	        "c_size",
	        TypeInfo{
	            .name = "c_size",
	            .mangledName = "c_size",
	            .calculatedSize = 8,
	            .isScalar = true,
	            .platformDependent = true,
	        },
	    }, // c_size
	    {
	        "c_voidptr",
	        TypeInfo{
	            .name = "c_voidptr",
	            .mangledName = "c_voidptr",
	            .calculatedSize = 8,
	            .isScalar = true,
	            .platformDependent = true,
	        },
	    }, // c_char
	};
	std::string key{lexeme};
	return map.contains(key) ? std::optional<TypeInfo>(map.at(key))
	                         : std::nullopt;
}
} // namespace ray::compiler::types
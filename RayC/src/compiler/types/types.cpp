#pragma once
#include <optional>
#include <unordered_map>

#include <ray/compiler/types/types.hpp>

namespace ray::compiler::types {
std::optional<TypeInfo>
TypeInfo::findScalarTypeInfo(const std::string_view lexeme) {
	static std::unordered_map<std::string, TypeInfo> map = {
	    {
	        "s32",
	        TypeInfo{
	            .name = "s32",
	            .mangledName = "s32",
	            .isScalar = true,
	            .calculatedSize = 4,
	        },
	    }, // s32
	};
	std::string key{lexeme};
	return map.contains(key) ? std::optional<TypeInfo>(map.at(key))
	                         : std::nullopt;
}
} // namespace ray::compiler::types
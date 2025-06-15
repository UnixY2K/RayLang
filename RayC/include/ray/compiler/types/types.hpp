#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace ray::compiler::types {
struct TypeInfo {
	std::string name;
	std::string mangledName;
	size_t calculatedSize;
	bool isScalar;
	bool platformDependent;

	static std::optional<TypeInfo> findScalarTypeInfo(const std::string_view);
};
} // namespace ray::compiler::types
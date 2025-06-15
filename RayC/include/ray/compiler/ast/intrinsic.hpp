#pragma once

#include <string_view>

namespace ray::compiler::ast {
enum class IntrinsicType {
	INTR_SIZEOF,
	INTR_UNKNOWN,
};

IntrinsicType getintrinsicType(const std::string_view lexeme);
} // namespace ray::compiler::ast
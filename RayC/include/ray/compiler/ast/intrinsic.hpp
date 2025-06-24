#pragma once

#include <string_view>

namespace ray::compiler::ast {
enum class IntrinsicType {
	INTR_SIZEOF,		//@sizeOf(myType)
	INTR_IMPORT,		//@import("myPackage:dir/file.ray")
	INTR_UNKNOWN,		// unrecognized intrinsic
};

IntrinsicType getintrinsicType(const std::string_view lexeme);
} // namespace ray::compiler::ast
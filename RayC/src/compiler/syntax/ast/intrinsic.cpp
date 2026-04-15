#include <string>
#include <unordered_map>

#include <ray/compiler/syntax/ast/intrinsic.hpp>

namespace ray::compiler::syntax::ast {

IntrinsicType getintrinsicType(const std::string_view lexeme) {
	static std::unordered_map<std::string, IntrinsicType> map = {
	    {"@sizeOf", IntrinsicType::INTR_SIZEOF}, // @sizeOf
	    {"@import", IntrinsicType::INTR_IMPORT}, // @import
	};
	std::string key{lexeme};
	return map.contains(key) ? map.at(key) : IntrinsicType::INTR_UNKNOWN;
}

} // namespace ray::compiler::syntax::ast
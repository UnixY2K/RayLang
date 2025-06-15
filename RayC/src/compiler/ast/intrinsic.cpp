#include <string>
#include <unordered_map>

#include <ray/compiler/ast/intrinsic.hpp>

namespace ray::compiler::ast {

IntrinsicType getintrinsicType(const std::string_view lexeme) {
	static std::unordered_map<std::string, ray::compiler::ast::IntrinsicType>
	    map = {
	        {"@sizeOf", IntrinsicType::INTR_SIZEOF}, // @sizeOf
	    };
	std::string key{lexeme};
	return map.contains(key) ? map.at(key) : IntrinsicType::INTR_UNKNOWN;
}

} // namespace ray::compiler::ast
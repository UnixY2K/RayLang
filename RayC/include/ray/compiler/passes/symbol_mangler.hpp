#pragma once
#include <optional>
#include <string>
#include <string_view>

#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>

namespace ray::compiler::passes::mangling {
class NameMangler {
	constexpr static std::string_view manglerVersion = "0";

  public:
	std::string mangleFunction(
	    std::string_view module, const ast::Function &function,
	    std::optional<directive::LinkageDirective> &linkageDirective);
	std::string
	mangleStruct(std::string_view module, const ast::Struct &structDefinition,
	             std::optional<directive::LinkageDirective> &linkageDirective);
};
} // namespace ray::compiler::passes::mangling

#pragma once
#include <optional>
#include <string>
#include <string_view>

#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>

namespace ray::compiler::passes::mangling {
class NameMangler {
	constexpr static std::string_view manglerVersion = "0";

  public:
	std::string mangleFunction(
	    std::string_view module, const syntax::ast::Function &function,
	    std::optional<directive::LinkageDirective> &linkageDirective);
	std::string
	mangleMethod(std::string_view module, const syntax::ast::Method &method,
	             std::optional<directive::LinkageDirective> &linkageDirective);
	std::string
	mangleStruct(std::string_view module,
	             const syntax::ast::Struct &structDefinition,
	             std::optional<directive::LinkageDirective> &linkageDirective);
	std::string mangleTrait(std::string_view module,
	                        const syntax::ast::Trait &traitDefinition);
};
} // namespace ray::compiler::passes::mangling

#include <format>

#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/generators/c/c_mangler.hpp>

namespace ray::compiler::generator::c {
std::string NameMangler::mangleFunction(
    std::string_view module, std::string_view namespacePath,
    const ast::Function &function,
    std::optional<directive::LinkageDirective> &linkageDirective) {
	if (linkageDirective) {
		if (!linkageDirective->overrideName.empty()) {
			return linkageDirective->overrideName;
		}
		if (linkageDirective->mangling ==
		    directive::LinkageDirective::ManglingType::C) {
			return function.name.lexeme;
		}
	}
	return std::format("_Ray{}{}{}{}{}{}{}{}", manglerVersion, "F",
	                   module.size(), module, namespacePath.size(),
	                   namespacePath, function.name.lexeme.size(),
	                   function.name.lexeme);
}
} // namespace ray::compiler::generator::c

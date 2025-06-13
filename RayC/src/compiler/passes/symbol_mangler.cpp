#include <format>
#include <iostream>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>

namespace ray::compiler::passes::mangling {
using namespace terminal::literals;

std::string NameMangler::mangleFunction(
    std::string_view module, std::string_view namespacePath,
    const ast::Function &function,
    std::optional<directive::LinkageDirective> &linkageDirective) {
	if (linkageDirective) {
		if (!linkageDirective->overrideName.empty()) {
			return linkageDirective->overrideName;
		}
		switch (linkageDirective->mangling) {
		case directive::LinkageDirective::ManglingType::Default: {
			break;
		}
		case directive::LinkageDirective::ManglingType::C: {
			return function.name.lexeme;
		}
		case directive::LinkageDirective::ManglingType::Unknonw: {
			// the ideal would be to return an optional
			// and make the compilation to fail
			std::cerr << std::format(
			    "{}: unknown linkage directive, using Default",
			    "WARNING"_yellow);
			break;
		}
		}
	}
	return std::format("_Ray{}_{}_{}_{}_{}_{}_{}_{}", manglerVersion, "F",
	                   module.size(), module, namespacePath.size(),
	                   namespacePath, function.name.lexeme.size(),
	                   function.name.lexeme);
}
} // namespace ray::compiler::passes::mangling

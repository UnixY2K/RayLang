#include <format>
#include <iostream>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>

namespace ray::compiler::passes::mangling {
using namespace terminal::literals;

std::string NameMangler::mangleFunction(
    std::string_view module, const ast::Function &function,
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
	return std::format("_RayMv{}_T{}_M{}_{}_N{}_{}", manglerVersion, "F",
	                   module.size(), module, function.name.lexeme.size(),
	                   function.name.lexeme);
}
std::string NameMangler::mangleStruct(
    std::string_view module, const ast::Struct &structDefinition,
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
			return structDefinition.name.lexeme;
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
	return std::format("_RayMv{}_T{}_M{}_{}_N{}_{}", manglerVersion, "S",
	                   module.size(), module,
	                   structDefinition.name.lexeme.size(),
	                   structDefinition.name.lexeme);
}
} // namespace ray::compiler::passes::mangling

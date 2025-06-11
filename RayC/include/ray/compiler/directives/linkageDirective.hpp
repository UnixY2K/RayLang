#pragma once
#include <string>
#include <string_view>

#include <ray/compiler/directives/compilerDirective.hpp>

namespace ray::compiler::directive {

class LinkageDirective : public CompilerDirective {
  public:
	enum class ManglingType {
		Default,
		C,
		Unknonw
	};

	std::string overrideName;
	bool isExternal = false;
	ManglingType mangling = ManglingType::Default;

	LinkageDirective(std::string overrideName, bool isExternal,
	                 ManglingType mangling)
	    : overrideName{overrideName}, isExternal{isExternal},
	      mangling{mangling} {}

	std::string_view directiveName() override { return "Linkage"; };

};

} // namespace ray::compiler::directive

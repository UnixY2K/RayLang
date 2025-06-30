#pragma once
#include <string>
#include <string_view>

#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler::directive {

class LinkageDirective : public CompilerDirective {
  public:
	enum class ManglingType { Default, C, Unknonw };

	std::string overrideName;
	bool isExternal = false;
	ManglingType mangling = ManglingType::Default;
	Token token;

	LinkageDirective(std::string overrideName, bool isExternal,
	                 ManglingType mangling, Token token)
	    : overrideName{overrideName}, isExternal{isExternal},
	      mangling{mangling}, token(token) {}

	const Token &getToken() const override { return token; }

	std::string_view directiveName() const override { return "Linkage"; };
};

} // namespace ray::compiler::directive

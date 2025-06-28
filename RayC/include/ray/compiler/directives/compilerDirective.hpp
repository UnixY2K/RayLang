#pragma once

#include <string_view>

#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler::directive {

class CompilerDirective {
  public:
	virtual std::string_view directiveName() const = 0;
	virtual const Token &getToken() const = 0;
	virtual ~CompilerDirective() = default;
};

} // namespace ray::compiler::directive

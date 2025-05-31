#pragma once

#include <string_view>

namespace ray::compiler::directive {

class CompilerDirective {
  public:
	virtual std::string_view directiveName() = 0;
	virtual ~CompilerDirective() = default;
};

} // namespace ray::compiler::directive

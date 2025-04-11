#pragma once
#include <cstddef>
#include <stdexcept>
#include <string_view>

#include <ray/compiler/lexer/token.hpp>
namespace ray::compiler {

class RuntimeError : public std::runtime_error {
  public:
	Token token;

	RuntimeError(const Token &token, std::string_view message)
	    : std::runtime_error(std::string(message).c_str()), token(token) {}
};

class ErrorBag {
  public:
	static bool hadError;

	static void error(size_t line, size_t column, std::string_view message);

	static void error(const Token token, std::string_view message);

  private:
	static void report(size_t line, size_t column, std::string_view where,
	                   std::string_view message);
};

} // namespace ray::compiler

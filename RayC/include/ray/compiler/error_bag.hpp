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
	bool hadError = false;
	std::string filepath;

  public:
	ErrorBag(std::string filepath) : filepath(filepath) {};

	bool failed() const;

	void error(size_t line, size_t column, std::string_view message);

	void error(const Token token, std::string_view message);

  private:
	void report(size_t line, size_t column, std::string_view where,
	            std::string_view message);
};

} // namespace ray::compiler

#pragma once
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <ray/compiler/lexer/token.hpp>
namespace ray::compiler {

class RuntimeError : public std::runtime_error {
  public:
	Token token;

	RuntimeError(const Token &token, std::string_view message)
	    : std::runtime_error(std::string(message).c_str()), token(token) {}
};

class MessageBag {
	std::string filepath;

	std::vector<std::string> errors;
	std::vector<std::string> warnings;

  public:
	MessageBag(std::string filepath) : filepath(filepath) {};

	void error(size_t line, size_t column, std::string_view category,
	           std::string_view message);

	void error(const Token token, std::string_view category,
	           std::string_view message);

	void warning(size_t line, size_t column, std::string_view category,
	             std::string_view message);

	void warning(const Token token, std::string_view category,
	             std::string_view message);

	bool failed() const;
	const std::vector<std::string> getErrors() const;
	const std::vector<std::string> getWarnings() const;

  private:
	void reportError(size_t line, size_t column, std::string_view where,
	                 std::string_view category, std::string_view message);
	void reportWarning(size_t line, size_t column, std::string_view where,
	                   std::string_view category, std::string_view message);
};

} // namespace ray::compiler

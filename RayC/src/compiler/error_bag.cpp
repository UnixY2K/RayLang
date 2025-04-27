#include <cstddef>
#include <ray/cli/terminal.hpp>
#include <ray/compiler/error_bag.hpp>

#include <format>
#include <iostream>

namespace ray::compiler {
using namespace ray::compiler::terminal::literals;

bool ErrorBag::failed() const { return hadError; }

void ErrorBag::error(size_t line, size_t column, std::string_view message) {
	report(line, column, "", message);
}

void ErrorBag::error(const Token token, std::string_view message) {
	if (token.type == Token::TokenType::TOKEN_EOF) {
		report(token.line, token.column, " at end", message);
	} else {
		report(token.line, token.column,
		       std::format("at '{}'", token.getLexeme()), message);
	}
}

void ErrorBag::report(size_t line, size_t column, std::string_view where,
                      std::string_view message) {
	std::cerr << std::format("{} [{}:{}:{}] {}: {}\n", "Error"_red, filepath,
	                         line, column, where, message);
	hadError = true;
}
} // namespace ray::compiler

#include <cstddef>
#include <ray/compiler/error_bag.hpp>

#include <format>
#include <iostream>

namespace ray::compiler {
bool ErrorBag::hadError = false;

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
	std::cerr << std::format("[{}: {}] Error {}: {}\n", line, column, where,
	                         message);
	hadError = true;
}
} // namespace ray::compiler

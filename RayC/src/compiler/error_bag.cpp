#include <cstddef>
#include <ray/cli/terminal.hpp>
#include <ray/compiler/error_bag.hpp>

#include <format>
#include <string_view>
#include <vector>

namespace ray::compiler {
using namespace ray::compiler::terminal::literals;

void ErrorBag::error(size_t line, size_t column, std::string_view category,
                     std::string_view message) {
	report(line, column, "", category, message);
}

void ErrorBag::error(const Token token, std::string_view category,
                     std::string_view message) {
	if (token.type == Token::TokenType::TOKEN_EOF) {
		report(token.line, token.column, " at end", category, message);
	} else {
		report(token.line, token.column,
		       std::format("at '{}'", token.getLexeme()), category, message);
	}
}

bool ErrorBag::failed() const { return !errors.empty(); }
const std::vector<std::string> ErrorBag::getErrors() const { return errors; }

void ErrorBag::report(size_t line, size_t column, std::string_view where,
                      std::string_view category, std::string_view message) {
	errors.push_back(std::format("{}|{} [{}:{}:{}] {}: {}\n", "Error"_red,
	                             terminal::red(category), filepath, line,
	                             column, where, message));
}
} // namespace ray::compiler

#include <cstddef>
#include <format>
#include <string_view>
#include <vector>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/message_bag.hpp>

namespace ray::compiler {
using namespace ray::compiler::terminal::literals;

void MessageBag::error(size_t line, size_t column, std::string_view category,
                       std::string_view message) {
	reportError(line, column, "", category, message);
}

void MessageBag::error(const Token token, std::string_view category,
                       std::string_view message) {
	if (token.type == Token::TokenType::TOKEN_EOF) {
		reportError(token.line, token.column, " at end", category, message);
	} else {
		reportError(token.line, token.column,
		            std::format("at '{}'", token.getLexeme()), category,
		            message);
	}
}

void MessageBag::warning(size_t line, size_t column, std::string_view category,
                         std::string_view message) {
	reportError(line, column, "", category, message);
}

void MessageBag::warning(const Token token, std::string_view category,
                         std::string_view message) {
	if (token.type == Token::TokenType::TOKEN_EOF) {
		reportWarning(token.line, token.column, " at end", category, message);
	} else {
		reportWarning(token.line, token.column,
		              std::format("at '{}'", token.getLexeme()), category,
		              message);
	}
}

void MessageBag::bug(size_t line, size_t column, std::string_view category,
                     std::string_view message) {
	reportBug(line, column, "", category, message);
}

void MessageBag::bug(const Token token, std::string_view category,
                     std::string_view message) {
	if (token.type == Token::TokenType::TOKEN_EOF) {
		reportBug(token.line, token.column, " at end", category, message);
	} else {
		reportBug(token.line, token.column,
		          std::format("at '{}'", token.getLexeme()), category, message);
	}
}

bool MessageBag::failed() const { return !errors.empty(); }
const std::vector<std::string> MessageBag::getErrors() const { return errors; }
const std::vector<std::string> MessageBag::getWarnings() const {
	return warnings;
}

void MessageBag::reportError(size_t line, size_t column, std::string_view where,
                             std::string_view category,
                             std::string_view message) {
	errors.push_back(std::format("{}|{} [{}:{}:{}] {}: {}\n", "Error"_red,
	                             terminal::red(category), filepath, line,
	                             column, where, message));
}
void MessageBag::reportWarning(size_t line, size_t column,
                               std::string_view where,
                               std::string_view category,
                               std::string_view message) {
	errors.push_back(std::format("{}|{} [{}:{}:{}] {}: {}\n", "Warning"_yellow,
	                             terminal::red(category), filepath, line,
	                             column, where, message));
}
void MessageBag::reportBug(size_t line, size_t column, std::string_view where,
                           std::string_view category,
                           std::string_view message) {
	errors.push_back(std::format("{}|{} [{}:{}:{}] {}: {}\n", "Bug"_red,
	                             terminal::red(category), filepath, line,
	                             column, where, message));
}
} // namespace ray::compiler

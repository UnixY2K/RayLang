#include <ray/compiler/lexer/lexer_error.hpp>
#include <ray/compiler/lexer/token.hpp>

#include <format>
#include <string_view>

namespace ray::compiler {
std::string LexerError::toString() const {
	switch (category) {
	case ErrorCategory::UnterminatedString: {
		return std::format("Unterminated string at line: {}, character: {}",
		                   token.line,
		                   token.column + token.lexeme.length() - 1);
	}
	case ErrorCategory::UnexpectedCharacter:
		return std::format(
		    "Unexpected character: '{}' at line: {}, character: {}",
		    token.lexeme, token.line, token.column);
	case ErrorCategory::Undefined:
	case ErrorCategory::Unkown:
		return std::format("[{}]{} at line: {},character: {}", "", message,
		                   token.line, token.column);
	}
}

std::string_view
LexerError::ErrorCategoryName(LexerError::ErrorCategory error) {
	switch (error) {
	case ErrorCategory::Undefined:
		return "Undefined";
	case ErrorCategory::UnterminatedString:
		return "UnterminatedString";
	case ErrorCategory::UnexpectedCharacter:
		return "UnexpectedCharacter";
	case ErrorCategory::Unkown:
		return "Unknown";
	}
	// should never show, if it does is a bug in the switch bellow
	return "UNKNOWN_TAG";
}

} // namespace ray::compiler

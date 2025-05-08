#include <ray/compiler/lexer/lexer_error.hpp>
#include <ray/compiler/lexer/token.hpp>

#include <format>
#include <string_view>

namespace ray::compiler {
std::string LexerError::toString() const {
	switch (category) {
	case ErrorCategory::UnterminatedString: {
		return std::format("Unterminated string");
	}
	case ErrorCategory::UnexpectedCharacter:
		return std::format("Unexpected character: '{}'", token.lexeme);
	case ErrorCategory::Undefined:
	case ErrorCategory::Unknown:
		return std::format("|Unknown/Undefined|: {} ", message);
	default:
		return std::format("|UNHANDLED-CAT| {}", message);
	}
}
std::string LexerError::positionString() const {
	switch (category) {
	case ErrorCategory::UnterminatedString: {
		return std::format("{}:{}", token.line,
		                   token.column + token.lexeme.length() - 1);
	}
	case ErrorCategory::UnexpectedCharacter:
		return std::format("{}:{}", token.line, token.column);
	case ErrorCategory::Undefined:
	case ErrorCategory::Unknown:
		return std::format("{}:{}", token.line, token.column);
	default:
		return std::format("{}:{}", token.line, token.column);
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
	case ErrorCategory::Unknown:
		return "Unknown";
	}
	// should never show, if it does is a bug in the switch bellow
	return "UNKNOWN_TAG";
}

} // namespace ray::compiler

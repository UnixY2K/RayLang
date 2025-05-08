#pragma once
#include <ray/compiler/lexer/token.hpp>

#include <string_view>

namespace ray::compiler {
class LexerError {
  public:
	enum class ErrorCategory{
		Undefined,
		UnterminatedString,
		UnexpectedCharacter,
		Unknown,
	};
	ErrorCategory category;
	Token token;
	std::string message;

	std::string toString() const;
	std::string positionString() const;

	static std::string_view ErrorCategoryName(ErrorCategory error);
};
} // namespace ray::compiler

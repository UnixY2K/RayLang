#pragma once

#include <cstddef>
#include <ray/compiler/lexer/token.hpp>

#include <vector>

namespace ray::compiler {

class Parser {
	std::vector<Token> tokens;
	size_t current = 0;

  public:
	Parser() = default;
	Parser(std::vector<Token> tokens) : tokens(tokens) {}

	std::vector<Statement> parse();
};
} // namespace ray::compiler

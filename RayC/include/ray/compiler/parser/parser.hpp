#pragma once
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/ast/statement.hpp>

#include <cstddef>
#include <vector>

namespace ray::compiler {

class Parser {
	std::vector<Token> tokens;
	size_t current = 0;

  public:
	Parser() = default;
	Parser(std::vector<Token> tokens) : tokens(tokens) {}

	std::vector<ast::Statement> parse();
};
} // namespace ray::compiler

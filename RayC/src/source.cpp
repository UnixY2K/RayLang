#include <format>
#include <iostream>

#include <ray/compiler/lexer/token.hpp>

using namespace ray::compiler;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << std::format("Usage: {} <name>\n", argv[0]);
		return 1;
	}

	Token token{Token::fromString("{"), "{", 0};

	std::cout << std::format("{}\n", token.toString());
}

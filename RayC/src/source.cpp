#include "ray/compiler/parser/parser.hpp"
#include <format>
#include <fstream>
#include <iostream>

#include <ray/cli/cli_args.hpp>
#include <ray/cli/terminal.hpp>

#include <ray/compiler/lexer/lexer.hpp>
#include <sstream>

using namespace ray::compiler;

using namespace ray::compiler::terminal::literals;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << std::format("Usage: {} <name>\n", argv[0]);
		return 1;
	}

	auto result = ray::compiler::cli::parse_args(argc, argv);
	if (std::holds_alternative<std::vector<std::string>>(result)) {
		for (const auto &error : std::get<std::vector<std::string>>(result)) {
			std::cout << error << '\n';
		}
		return 1;
	} else {
		auto opts = std::get<ray::compiler::cli::Options>(result);
		if (!opts.validate()) {
			return 1;
		}

		// read the contents of the file
		std::ifstream input(opts.input);
		if (!input) {
			std::cerr << std::format("{}: could not open file: {}\n",
			                         "Error"_red, opts.input.string());
			return 1;
		}
		std::ostringstream oss{};
		oss << input.rdbuf();

		Lexer lexer(oss.view());

		auto tokens = lexer.scanTokens();
		if (lexer.getErrors().size() == 0) {
			auto parser = Parser(tokens);
			auto statements = parser.parse();
		} else {
			for (auto &error : lexer.getErrors()) {
				std::cerr << std::format("{}: {}\n", "LexerError"_red,
				                         error.toString());
			}
		}
	}
}

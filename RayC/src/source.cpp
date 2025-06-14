#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

#include <ray/cli/cli_args.hpp>
#include <ray/cli/options.hpp>
#include <ray/cli/terminal.hpp>

#include <ray/compiler/passes/resolver.hpp>

#include <ray/compiler/generators/c/c_transpiler.hpp>
#include <ray/compiler/generators/wasm/wasm_text.hpp>

#include <ray/compiler/lexer/lexer.hpp>
#include <ray/compiler/parser/parser.hpp>

using namespace ray::compiler;

using namespace ray::compiler::terminal::literals;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << std::format("Usage: {} <name>\n", argv[0]);
		return 1;
	}

	try {
		auto result = ray::compiler::cli::parse_args(argc, argv);
		if (std::holds_alternative<std::vector<std::string>>(result)) {
			for (const auto &error :
			     std::get<std::vector<std::string>>(result)) {
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
			if (lexer.getErrors().size() > 0) {
				for (auto &error : lexer.getErrors()) {
					std::cerr << std::format(
					    "{}: [{}:{}] {}\n", "LexerError"_red,
					    opts.input.string(), error.positionString(),
					    error.toString());
				}
				return 1;
			}

			auto parser = Parser(opts.input.relative_path().string(), tokens);
			auto statements = parser.parse();
			if (parser.failed()) {
				return 1;
			}
			if (opts.target == ray::compiler::cli::Options::TargetEnum::NONE) {
				opts.target = opts.defaultTarget;
			}
			std::string output;
			bool handled = false;
			analyzer::symbols::Resolver symbolTableGen;
			symbolTableGen.resolve(statements);

			if (symbolTableGen.hasFailed()) {
				std::cerr << std::format("{}: {}\n", "Error"_red,
				                         "symbolTableGen failed");
				return 1;
			}
			switch (opts.target) {
			case cli::Options::TargetEnum::WASM_TEXT: {
				handled = true;
				generator::wasm::WASMTextGenerator wasmTextGen;

				wasmTextGen.resolve(statements);
				if (wasmTextGen.hasFailed()) {
					std::cerr << std::format("{}: {}\n", "Error"_red,
					                         "WASMTextGenerator failed");
					return 1;
				}
				output = wasmTextGen.getOutput();
			}
			case cli::Options::TargetEnum::C_SOURCE: {
				handled = true;
				generator::c::CTranspilerGenerator CTranspilerGen;

				CTranspilerGen.resolve(statements, symbolTableGen.getSymbolTable());
				if (CTranspilerGen.hasFailed()) {
					std::cerr << std::format("{}: {}\n", "Error"_red,
					                         "CSourceGen failed");
					return 1;
				}
				output = CTranspilerGen.getOutput();
			}
			// both cases should never show
			case cli::Options::TargetEnum::NONE:
			case cli::Options::TargetEnum::ERROR:
				break;
			}
			if (!handled) {
				std::cerr << std::format(
				    "{}: unhandled target option, this is a compiler bug\n",
				    "COMPILER-ERROR"_red);
				return -1;
			}

			std::ofstream outputFile(opts.output, std::ios::trunc);
			if (!outputFile) {
				std::cerr << std::format("{}: could not open file: {}\n",
				                         "Error"_red, opts.output.string());
				return 1;
			}
			outputFile << output;
		}

	} catch (std::exception &ex) {
		std::cerr << std::format("{}: {}", "UNHANDLED_ERROR"_red, ex.what());
		return -1;
	}
}

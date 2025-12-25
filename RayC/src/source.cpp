#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

#include <ray/cli/cli_args.hpp>
#include <ray/cli/options.hpp>
#include <ray/cli/terminal.hpp>

#include <ray/compiler/environment/dataModel/dataModel.hpp>

#include <ray/compiler/lexer/lexer.hpp>
#include <ray/compiler/parser/parser.hpp>

#include <ray/compiler/passes/typeChecker.hpp>
#include <ray/compiler/passes/typeScanner.hpp>

#include <ray/compiler/generators/c/c_transpiler.hpp>

#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>

// wingdi.h is included somewhere and is defining ERROR and as macro...
#ifdef ERROR
#undef ERROR
#endif

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

			const environment::DataModel *dataModel;
			switch (opts.dataModel) {
			case ray::compiler::cli::Options::TargetDataModel::NONE: {
				std::cerr << std::format("{}: no data model available\n",
				                         "Error"_red);
				return 1;
			}
			case ray::compiler::cli::Options::TargetDataModel::LLP64: {
				dataModel = &environment::DataModel::LLP64DataModel();
				break;
			}
			case ray::compiler::cli::Options::TargetDataModel::LP64: {
				dataModel = &environment::DataModel::LP64DataModel();
				break;
			}
			default: {
				std::cerr << std::format(
				    "{}: the selected data model is not supported\n",
				    "Error"_red);
				return 1;
			}
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

			auto sourceFile =
			    opts.input.make_preferred().relative_path().string();
			auto parser = Parser(sourceFile, tokens);
			auto statements = parser.parse();

			if (parser.failed()) {
				for (auto parseError : parser.getErrors()) {
					std::cerr << parseError;
				}
				return 1;
			}
			if (opts.target == ray::compiler::cli::Options::TargetEnum::NONE) {
				opts.target = opts.defaultTarget;
			}

			std::string output;
			bool handled = false;

			lang::ModuleStore moduleStore;

			passes::TypeScanner typeScanner(sourceFile, *dataModel);

			typeScanner.resolve(statements);
			// TODO: once a propper typeScanner is set in place replace this so
			// type checker errors can be reported along with the previous
			// errors
			if (typeScanner.hasFailed()) {
				std::cerr << std::format("{}: {}\n", "Error"_red,
				                         "typeChecker failed");
				for (auto typeScannerError : typeScanner.getErrors()) {
					std::cerr << typeScannerError;
				}
				return 1;
			}

			passes::TypeChecker typeChecker(sourceFile, moduleStore,
			                                *dataModel);

			typeChecker.resolve(statements);
			if (typeChecker.hasFailed()) {
				std::cerr << std::format("{}: {}\n", "Error"_red,
				                         "typeChecker failed");
				for (auto typeCheckerError : typeChecker.getErrors()) {
					std::cerr << typeCheckerError;
				}
				return 1;
			}
			for (auto typeCheckerWarning : typeChecker.getWarnings()) {
				std::cerr << typeCheckerWarning;
			}

			switch (opts.target) {
			case cli::Options::TargetEnum::C_SOURCE: {
				handled = true;
				generator::c::CTranspilerGenerator CTranspilerGen(
				    sourceFile, typeChecker.getCurrentSourceUnit(), *dataModel);

				CTranspilerGen.resolve(statements);
				if (CTranspilerGen.hasFailed()) {
					std::cerr << std::format("{}: {}\n", "Error"_red,
					                         "CSourceGen failed");
					for (auto cError : CTranspilerGen.getErrors()) {
						std::cerr << cError;
					}
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
		std::cerr << std::format("{}: {}\n", "UNHANDLED_ERROR"_red, ex.what());
		return -1;
	}
}

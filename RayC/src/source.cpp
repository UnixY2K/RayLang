
#include <exception>
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

#include <ray/cli/cli_args.hpp>
#include <ray/cli/options.hpp>
#include <ray/cli/terminal.hpp>

#include <ray/compiler/environment/dataModel/dataModel.hpp>

#include <ray/compiler/infrastructure/compilationContext.hpp>
#include <ray/compiler/infrastructure/diagnosticsEngine.hpp>

#include <ray/compiler/lexer/lexer.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/parser/parser.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

#include <ray/compiler/passes/passManager.hpp>
#include <ray/compiler/passes/resolver.hpp>
#include <ray/compiler/passes/rst/typeChecker.hpp>
#include <ray/compiler/passes/rst/typeScanner.hpp>

#include <ray/compiler/backend/c/c_transpiler.hpp>

#include <ray/compiler/lang/module.hpp>
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
		if (!result) {
			for (const auto &error : result.error()) {
				std::cout << error << '\n';
			}
			return 1;
		}

		auto opts = result.value();

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
			    "{}: the selected data model is not supported\n", "Error"_red);
			return 1;
		}
		}
		auto sourceFile = opts.input.make_preferred().relative_path().string();
		if (opts.target == ray::compiler::cli::Options::TargetEnum::NONE) {
			opts.target = opts.defaultTarget;
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
				    "{}: [{}:{}] {}\n", "LexerError"_red, opts.input.string(),
				    error.positionString(), error.toString());
			}
			return 1;
		}

		auto parser = Parser(sourceFile, tokens);
		auto rootBlock = parser.parse();

		if (parser.failed()) {
			for (auto parseError : parser.getErrors()) {
				std::cerr << parseError;
			}
			return 1;
		}

		std::string output;
		bool handled = false;

		infrastructure::diagnostics::DiagnosticEngine diagnostics;
		infrastructure::CompilationContext compilationCtx{
		    lang::ModuleStore(), lang::SourceUnit(), *dataModel, diagnostics};

		passes::PassManager passManager;

		passManager.addPass<passes::Resolver>();
		passManager.addPass<passes::rst::TypeScanner>();

		auto finalCompilationArtifact = passManager.run(
		    compilationCtx, std::make_unique<infrastructure::CompilerArtifact>(
		                        infrastructure::CompilerArtifact(
		                            {std::move(rootBlock)}, std::nullopt)));

		if (compilationCtx.diagnostics.hasFailed()) {
			for (auto diagnostic :
			     compilationCtx.diagnostics.getDiagnostics()) {
				auto messageColor = terminal::Color::None;
				switch (diagnostic.severity) {

				case infrastructure::diagnostics::DiagnosticSeverity::Warning:
					messageColor = terminal::Color::Yellow;
					break;
				case infrastructure::diagnostics::DiagnosticSeverity::Error:
				case infrastructure::diagnostics::DiagnosticSeverity::Fatal:
				case infrastructure::diagnostics::DiagnosticSeverity::Bug:
					messageColor = terminal::Color::Red;
					break;
				}
				// TODO: set correctly filePath based off
				// diagnostic.location.sourceId
				std::string filePath = opts.input.string();
				const auto &location = diagnostic.location;
				std::cerr << std::format(
				    "{}|{} [{}:{}:{}] : {}\n",
				    terminal::colored(diagnostic.severityAsString(),
				                      messageColor),
				    terminal::colored(diagnostic.category, messageColor),
				    filePath, location.line, location.column,
				    diagnostic.message);
			}
			return 1;
		}

		// TODO: remove this block once all the phases use the pass manager
		auto defaultRSTBlock = syntax::rst::Block({}, Token::makeEOFToken());
		auto &finalRSTBlock = *finalCompilationArtifact->rootRSTBlock
		                           .transform([](auto &blockUniquePtr) {
			                           return blockUniquePtr.get();
		                           })
		                           .value_or(&defaultRSTBlock);

		lang::ModuleStore moduleStore;
		lang::SourceUnit sourceUnit;

		passes::rst::TypeChecker typeChecker(sourceFile, moduleStore,
		                                     *dataModel, sourceUnit);

		typeChecker.resolve(finalRSTBlock);
		if (typeChecker.hasFailed()) {
			std::cerr << std::format("{}: {}\n", "Error"_red,
			                         "typeChecker failed");
			for (auto typeCheckerError :
			     typeChecker.getMessageBag().getErrors()) {
				std::cerr << typeCheckerError;
			}
			return 1;
		}
		for (auto typeCheckerWarning :
		     typeChecker.getMessageBag().getWarnings()) {
			std::cerr << typeCheckerWarning;
		}

		switch (opts.target) {
		case cli::Options::TargetEnum::C_SOURCE: {
			handled = true;
			backend::c::CTranspilerGenerator CTranspilerGen(
			    sourceFile, typeChecker.getCurrentSourceUnit(), *dataModel);

			CTranspilerGen.resolve(finalRSTBlock);
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

	} catch (std::exception &ex) {
		std::cerr << std::format("{}: {}\n", "UNHANDLED_ERROR"_red, ex.what());
		return -1;
	}
}

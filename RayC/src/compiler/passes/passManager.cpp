#include <memory>
#include <sstream>

#include <ray/compiler/infrastructure/compilationContext.hpp>
#include <ray/compiler/lexer/lexer.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/parser/parser.hpp>
#include <ray/compiler/passes/passManager.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

namespace ray::compiler::passes {

std::unique_ptr<infrastructure::CompilerArtifact>
PassManager::run(infrastructure::CompilationContext &context,
                 std::unique_ptr<infrastructure::CompilerArtifact>
                     initialCompilationArtifact) {
	std::unique_ptr<infrastructure::CompilerArtifact> currentArtifact =
	    std::move(initialCompilationArtifact);

	for (auto &compilerPass : passes) {
		if (context.diagnostics.hasFatallyFailed() ||
		    (compilerPass->requiresCleanState() &&
		     context.diagnostics.hasFailed())) {
			break;
		}

		context.diagnostics.set_category(compilerPass->name());
		compilerPass->run(context, std::move(currentArtifact));
		currentArtifact = compilerPass->getCompilationArtifact();
	}

	return currentArtifact;
}

std::unique_ptr<infrastructure::CompilerArtifact>
PassManager::run(infrastructure::CompilationContext &context,
                 std::string_view filePath, std::string_view source) {
	// verify that the file exist
	context.diagnostics.set_category("lexer");
	// TODO: once we have a way to realiably make source IDs have this to
	// properly set the ID of the initial file even it will most likely be 0
	context.diagnostics.set_currentSourceID(0);
	Lexer lexer(context, source);
	auto tokens = lexer.scanTokens();

	// lets consider lexer and parse errors are not fatal so the rest of the
	// phases can still perform checks

	context.diagnostics.set_category("parser");
	auto parser = Parser(lexer.scanTokens());

	auto rootBlock = parser.parse();

	return run(context, std::make_unique<infrastructure::CompilerArtifact>(
	                        infrastructure::CompilerArtifact(
	                            {std::move(rootBlock)}, std::nullopt)));
}

} // namespace ray::compiler::passes
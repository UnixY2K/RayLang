#include <memory>
#include <ray/compiler/infrastructure/compilationContext.hpp>
#include <ray/compiler/lexer/token.hpp>
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

} // namespace ray::compiler::passes
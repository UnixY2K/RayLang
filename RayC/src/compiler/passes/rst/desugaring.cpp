#include <ray/compiler/passes/rst/desugaring.hpp>
#include <utility>

namespace ray::compiler::passes::rst {
void Desugaring::run(infrastructure::CompilationContext &ctx,
                     std::unique_ptr<infrastructure::CompilerArtifact>
                         previousCompilerArtifact) {
	compilationContext = &ctx;
	currentCompilerArtifact = std::move(previousCompilerArtifact);
}
std::unique_ptr<infrastructure::CompilerArtifact>
Desugaring::getCompilationArtifact() {
	return std::move(currentCompilerArtifact);
}
} // namespace ray::compiler::passes::rst
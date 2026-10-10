#include <ray/compiler/passes/rst/moduleResolver.hpp>

namespace ray::compiler::passes::rst {
void moduleResolver::run(infrastructure::CompilationContext &ctx,
                         std::unique_ptr<infrastructure::CompilerArtifact>
                             previousCompilerArtifact) {
	compilationContext = &ctx;
	currentCompilerArtifact = std::move(previousCompilerArtifact);
}
std::unique_ptr<infrastructure::CompilerArtifact>
moduleResolver::getCompilationArtifact() {
	return std::move(currentCompilerArtifact);
}
} // namespace ray::compiler::passes::rst
#include <ray/compiler/passes/rst/lowering.hpp>

namespace ray::compiler::passes::rst {
void Lowering::run(infrastructure::CompilationContext &ctx,
                   std::unique_ptr<infrastructure::CompilerArtifact>
                       previousCompilerArtifact) {
	compilationContext = &ctx;
	currentCompilerArtifact = std::move(previousCompilerArtifact);
}
std::unique_ptr<infrastructure::CompilerArtifact>
Lowering::getCompilationArtifact() {
	return std::move(currentCompilerArtifact);
}
} // namespace ray::compiler::passes::rst
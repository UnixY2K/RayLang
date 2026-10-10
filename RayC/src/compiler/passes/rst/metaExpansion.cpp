#include <ray/compiler/passes/rst/metaExpansion.hpp>

namespace ray::compiler::passes::rst {
void MetaExpansion::run(infrastructure::CompilationContext &ctx,
                        std::unique_ptr<infrastructure::CompilerArtifact>
                            previousCompilerArtifact) {
	compilationContext = &ctx;
	currentCompilerArtifact = std::move(previousCompilerArtifact);
}
std::unique_ptr<infrastructure::CompilerArtifact>
MetaExpansion::getCompilationArtifact() {
	return std::move(currentCompilerArtifact);
}
} // namespace ray::compiler::passes::rst
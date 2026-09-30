#pragma once

#include <ray/compiler/passes/compilerPass.hpp>

namespace ray::compiler::passes::rst {

class Lowering : public CompilerPass {
	infrastructure::CompilationContext *compilationContext;
	std::unique_ptr<infrastructure::CompilerArtifact> currentCompilerArtifact;
	bool requiresCleanState() const override { return true; }

  public:
	std::string_view name() const override { return "Lowering"; }
	void run(infrastructure::CompilationContext &ctx,
	         std::unique_ptr<infrastructure::CompilerArtifact>
	             previousCompilerArtifact) override;
	std::unique_ptr<infrastructure::CompilerArtifact>
	getCompilationArtifact() override;
};

} // namespace ray::compiler::passes::rst
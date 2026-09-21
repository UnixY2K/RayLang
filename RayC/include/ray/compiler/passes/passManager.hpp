
#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <ray/compiler/infrastructure/compilationContext.hpp>
#include <ray/compiler/passes/compilerPass.hpp>

namespace ray::compiler::passes {

class PassManager {
  private:
	std::vector<std::unique_ptr<CompilerPass>> passes;

  public:
	PassManager() = default;

	template <typename TPass, typename... Args> void addPass(Args &&...args) {
		passes.push_back(std::make_unique<TPass>(std::forward<Args>(args)...));
	}

	std::unique_ptr<infrastructure::CompilerArtifact>
	run(infrastructure::CompilationContext &context, std::unique_ptr<infrastructure::CompilerArtifact> initialCompilationArtifact);
	void clear() { passes.clear(); }
};

} // namespace ray::compiler::passes

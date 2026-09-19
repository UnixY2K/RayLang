#pragma once
#include <memory>

#include <ray/compiler/infrastructure/compilationContext.hpp>

namespace ray::compiler::passes {

class CompilerPass {
  public:
	virtual ~CompilerPass() = default;

	[[nodiscard]] virtual std::string_view name() const = 0;
	virtual bool run(infrastructure::CompilationContext &ctx) = 0;
	[[nodiscard]] virtual bool requiresCleanState() const { return false; }
	[[nodiscard]] virtual std::unique_ptr<infrastructure::CompilerArtifact>
	getCompilationArtifact() = 0;
};

} // namespace ray::compiler::passes

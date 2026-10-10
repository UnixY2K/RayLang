
#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
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

	template <class T>
	std::optional<std::reference_wrapper<T>> getCompilerPass() {
		for (const auto &pass : passes) {
			if (auto *derived = dynamic_cast<T *>(pass.get())) {
				return *derived;
			}
		}
		return std::nullopt;
	}

	std::unique_ptr<infrastructure::CompilerArtifact>
	run(infrastructure::CompilationContext &context,
	    std::unique_ptr<infrastructure::CompilerArtifact>
	        initialCompilationArtifact);
	std::unique_ptr<infrastructure::CompilerArtifact>
	run(infrastructure::CompilationContext &context, std::string_view filePath,
	    std::string_view source);
	void clear() { passes.clear(); }
};

} // namespace ray::compiler::passes

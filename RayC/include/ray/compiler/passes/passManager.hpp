
#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <ray/compiler/passes/compilerPass.hpp>

namespace ray::compiler::infrastructure {
struct CompilationContext;
}

namespace ray::compiler::passes {

class PassManager {
  private:
	std::vector<std::unique_ptr<CompilerPass>> passes;

  public:
	PassManager() = default;

	template <typename TPass, typename... Args> void addPass(Args &&...args) {
		passes.push_back(std::make_unique<TPass>(std::forward<Args>(args)...));
	}

	bool run(infrastructure::CompilationContext &ctx);
	void clear() { passes.clear(); }

	[[nodiscard]] const std::vector<std::unique_ptr<CompilerPass>> &
	getPasses() const {
		return passes;
	}
};

} // namespace ray::compiler::passes

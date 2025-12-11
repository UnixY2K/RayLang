#include <ray/compiler/passes/typeScanner.hpp>

namespace ray::compiler::passes {

void TypeScanner::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {}

bool TypeScanner::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TypeScanner::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TypeScanner::getWarnings() const {
	return messageBag.getWarnings();
}

} // namespace ray::compiler::analyzer
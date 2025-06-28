#include <ray/compiler/passes/typeChecker.hpp>

#include <format>

namespace ray::compiler::analyzer {

void TypeChecker::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement) {
	// globalStructDefinitions.clear();
	// globalFunctionDefinitions.clear();
	for (const auto &stmt : statement) {
		stmt->visit(*this);
	}

	for (auto &directive : directivesStack) {
		messageBag.warning(directive->getToken(), "TypeChecker",
		                   std::format("unused compiler directive {}",
		                               directive->directiveName()));
	}
}

bool TypeChecker::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TypeChecker::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TypeChecker::getWarnings() const {
	return messageBag.getWarnings();
}

void TypeChecker::visitBlockStatement(const ast::Block &value) {}
void TypeChecker::visitTerminalExprStatement(const ast::TerminalExpr &value) {}
void TypeChecker::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {}
void TypeChecker::visitFunctionStatement(const ast::Function &value) {}
void TypeChecker::visitIfStatement(const ast::If &value) {}
void TypeChecker::visitJumpStatement(const ast::Jump &value) {}
void TypeChecker::visitVarStatement(const ast::Var &value) {}
void TypeChecker::visitWhileStatement(const ast::While &value) {}
void TypeChecker::visitStructStatement(const ast::Struct &value) {}
void TypeChecker::visitCompDirectiveStatement(const ast::CompDirective &value) {
}
// Expression
void TypeChecker::visitAssignExpression(const ast::Assign &value) {}
void TypeChecker::visitBinaryExpression(const ast::Binary &value) {}
void TypeChecker::visitCallExpression(const ast::Call &value) {}
void TypeChecker::visitGetExpression(const ast::Get &value) {}
void TypeChecker::visitGroupingExpression(const ast::Grouping &value) {}
void TypeChecker::visitLiteralExpression(const ast::Literal &value) {}
void TypeChecker::visitLogicalExpression(const ast::Logical &value) {}
void TypeChecker::visitSetExpression(const ast::Set &value) {}
void TypeChecker::visitUnaryExpression(const ast::Unary &value) {}
void TypeChecker::visitArrayAccessExpression(const ast::ArrayAccess &value) {}
void TypeChecker::visitVariableExpression(const ast::Variable &value) {}
void TypeChecker::visitIntrinsicExpression(const ast::Intrinsic &value) {}
void TypeChecker::visitTypeExpression(const ast::Type &value) {}
void TypeChecker::visitCastExpression(const ast::Cast &value) {}
void TypeChecker::visitParameterExpression(const ast::Parameter &value) {}
} // namespace ray::compiler::analyzer
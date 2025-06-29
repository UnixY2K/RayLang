#include "ray/compiler/lexer/token.hpp"
#include <cstddef>
#include <format>

#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/passes/typeChecker.hpp>

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

void TypeChecker::visitBlockStatement(const ast::Block &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitTerminalExprStatement(const ast::TerminalExpr &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitFunctionStatement(const ast::Function &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitIfStatement(const ast::If &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitJumpStatement(const ast::Jump &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitVarStatement(const ast::Var &value) {
	auto type = lang::Type{};

	if (value.type.token.type != Token::TokenType::TOKEN_UNINITIALIZED) {
		// initialization code
	}

	size_t currentTop = typeStack.size();
	auto initializationType = lang::Type{};
	if (value.initializer.has_value()) {
		value.initializer.value()->visit(*this);
		if (currentTop > typeStack.size()) {
			initializationType = typeStack.back();
		}
	}

	if (!(type.isInitialized() || initializationType.isInitialized())) {
		messageBag.error(
		    value.getToken(), "TYPE-ERROR",
		    "variable does not have a type assigned nor an initialization");
	}
}
void TypeChecker::visitWhileStatement(const ast::While &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitStructStatement(const ast::Struct &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitCompDirectiveStatement(const ast::CompDirective &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
// Expression
void TypeChecker::visitAssignExpression(const ast::Assign &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitBinaryExpression(const ast::Binary &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitCallExpression(const ast::Call &value) {
	
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitGetExpression(const ast::Get &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitGroupingExpression(const ast::Grouping &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitLiteralExpression(const ast::Literal &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitLogicalExpression(const ast::Logical &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitSetExpression(const ast::Set &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitUnaryExpression(const ast::Unary &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitArrayAccessExpression(const ast::ArrayAccess &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitVariableExpression(const ast::Variable &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitTypeExpression(const ast::Type &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitCastExpression(const ast::Cast &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
void TypeChecker::visitParameterExpression(const ast::Parameter &value) {
	messageBag.error(value.getToken(), "BUG",
	                 std::format("visit method not implemented for {}",
	                             value.variantName()));
}
} // namespace ray::compiler::analyzer
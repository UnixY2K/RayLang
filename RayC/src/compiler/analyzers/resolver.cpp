#include <iostream>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/analyzers/resolver.hpp>

namespace ray::compiler::analyzer::symbols {
using namespace terminal::literals;

void Resolver::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement) {
	for (const auto &stmt : statement) {
		stmt->visit(*this);
	}

	if (!this->directivesStack.empty()) {
		for (auto &directive : directivesStack) {
			std::cerr << std::format("{}: unused compiler directive {}\n",
			                         "WARNING"_yellow,
			                         directive->directiveName());
		}
	}
}

void Resolver::visitBlockStatement(const ast::Block &value) {
	std::cerr << "visitBlockStatement not implemented\n";
}
void Resolver::visitTerminalExprStatement(
    const ast::TerminalExpr &value) {
	std::cerr << "visitTerminalExprStatement not implemented\n";
}
void Resolver::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	std::cerr << "visitExpressionStmtStatement not implemented\n";
}
void Resolver::visitFunctionStatement(const ast::Function &value) {
	std::cerr << "visitFunctionStatement not implemented\n";
}
void Resolver::visitIfStatement(const ast::If &value) {
	std::cerr << "visitIfStatement not implemented\n";
}
void Resolver::visitJumpStatement(const ast::Jump &value) {
	std::cerr << "visitJumpStatement not implemented\n";
}
void Resolver::visitVarStatement(const ast::Var &value) {
	std::cerr << "visitVarStatement not implemented\n";
}
void Resolver::visitWhileStatement(const ast::While &value) {
	std::cerr << "visitWhileStatement not implemented\n";
}
void Resolver::visitStructStatement(const ast::Struct &value) {
	std::cerr << "visitStructStatement not implemented\n";
}
void Resolver::visitNamespaceStatement(
    const ast::Namespace &value) {
	std::cerr << "visitNamespaceStatement not implemented\n";
}
void Resolver::visitExternStatement(const ast::Extern &value) {
	std::cerr << "visitExternStatement not implemented\n";
}
void Resolver::visitCompDirectiveStatement(
    const ast::CompDirective &value) {
	std::cerr << "visitCompDirectiveStatement not implemented\n";
}
// Expression
void Resolver::visitAssignExpression(const ast::Assign &value) {
	std::cerr << "visitAssignExpression not implemented\n";
}
void Resolver::visitBinaryExpression(const ast::Binary &value) {
	std::cerr << "visitBinaryExpression not implemented\n";
}
void Resolver::visitCallExpression(const ast::Call &value) {
	std::cerr << "visitCallExpression not implemented\n";
}
void Resolver::visitGetExpression(const ast::Get &value) {
	std::cerr << "visitGetExpression not implemented\n";
}
void Resolver::visitGroupingExpression(const ast::Grouping &value) {
	std::cerr << "visitGroupingExpression not implemented\n";
}
void Resolver::visitLiteralExpression(const ast::Literal &value) {
	std::cerr << "visitLiteralExpression not implemented\n";
}
void Resolver::visitLogicalExpression(const ast::Logical &value) {
	std::cerr << "visitLogicalExpression not implemented\n";
}
void Resolver::visitSetExpression(const ast::Set &value) {
	std::cerr << "visitSetExpression not implemented\n";
}
void Resolver::visitUnaryExpression(const ast::Unary &value) {
	std::cerr << "visitUnaryExpression not implemented\n";
}
void Resolver::visitArrayAccessExpression(
    const ast::ArrayAccess &value) {
	std::cerr << "visitArrayAccessExpression not implemented\n";
}
void Resolver::visitVariableExpression(const ast::Variable &value) {
	std::cerr << "visitVariableExpression not implemented\n";
}
void Resolver::visitTypeExpression(const ast::Type &value) {
	std::cerr << "visitTypeExpression not implemented\n";
}
void Resolver::visitCastExpression(const ast::Cast &value) {
	std::cerr << "visitCastExpression not implemented\n";
}
void Resolver::visitParameterExpression(
    const ast::Parameter &value) {
	std::cerr << "visitParameterExpression not implemented\n";
}

} // namespace ray::compiler::analyzer::symbols
#include <iostream>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/analyzers/symbolTableGenerator.hpp>

namespace ray::compiler::analyzer::symbols {
using namespace terminal::literals;

void SymbolTableGenerator::resolve(
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

void SymbolTableGenerator::visitBlockStatement(const ast::Block &value) {
	std::cerr << "visitBlockStatement not implemented\n";
}
void SymbolTableGenerator::visitTerminalExprStatement(
    const ast::TerminalExpr &value) {
	std::cerr << "visitTerminalExprStatement not implemented\n";
}
void SymbolTableGenerator::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	std::cerr << "visitExpressionStmtStatement not implemented\n";
}
void SymbolTableGenerator::visitFunctionStatement(const ast::Function &value) {
	std::cerr << "visitFunctionStatement not implemented\n";
}
void SymbolTableGenerator::visitIfStatement(const ast::If &value) {
	std::cerr << "visitIfStatement not implemented\n";
}
void SymbolTableGenerator::visitJumpStatement(const ast::Jump &value) {
	std::cerr << "visitJumpStatement not implemented\n";
}
void SymbolTableGenerator::visitVarStatement(const ast::Var &value) {
	std::cerr << "visitVarStatement not implemented\n";
}
void SymbolTableGenerator::visitWhileStatement(const ast::While &value) {
	std::cerr << "visitWhileStatement not implemented\n";
}
void SymbolTableGenerator::visitStructStatement(const ast::Struct &value) {
	std::cerr << "visitStructStatement not implemented\n";
}
void SymbolTableGenerator::visitNamespaceStatement(
    const ast::Namespace &value) {
	std::cerr << "visitNamespaceStatement not implemented\n";
}
void SymbolTableGenerator::visitExternStatement(const ast::Extern &value) {
	std::cerr << "visitExternStatement not implemented\n";
}
void SymbolTableGenerator::visitCompDirectiveStatement(
    const ast::CompDirective &value) {
	std::cerr << "visitCompDirectiveStatement not implemented\n";
}
// Expression
void SymbolTableGenerator::visitAssignExpression(const ast::Assign &value) {
	std::cerr << "visitAssignExpression not implemented\n";
}
void SymbolTableGenerator::visitBinaryExpression(const ast::Binary &value) {
	std::cerr << "visitBinaryExpression not implemented\n";
}
void SymbolTableGenerator::visitCallExpression(const ast::Call &value) {
	std::cerr << "visitCallExpression not implemented\n";
}
void SymbolTableGenerator::visitGetExpression(const ast::Get &value) {
	std::cerr << "visitGetExpression not implemented\n";
}
void SymbolTableGenerator::visitGroupingExpression(const ast::Grouping &value) {
	std::cerr << "visitGroupingExpression not implemented\n";
}
void SymbolTableGenerator::visitLiteralExpression(const ast::Literal &value) {
	std::cerr << "visitLiteralExpression not implemented\n";
}
void SymbolTableGenerator::visitLogicalExpression(const ast::Logical &value) {
	std::cerr << "visitLogicalExpression not implemented\n";
}
void SymbolTableGenerator::visitSetExpression(const ast::Set &value) {
	std::cerr << "visitSetExpression not implemented\n";
}
void SymbolTableGenerator::visitUnaryExpression(const ast::Unary &value) {
	std::cerr << "visitUnaryExpression not implemented\n";
}
void SymbolTableGenerator::visitArrayAccessExpression(
    const ast::ArrayAccess &value) {
	std::cerr << "visitArrayAccessExpression not implemented\n";
}
void SymbolTableGenerator::visitVariableExpression(const ast::Variable &value) {
	std::cerr << "visitVariableExpression not implemented\n";
}
void SymbolTableGenerator::visitTypeExpression(const ast::Type &value) {
	std::cerr << "visitTypeExpression not implemented\n";
}
void SymbolTableGenerator::visitCastExpression(const ast::Cast &value) {
	std::cerr << "visitCastExpression not implemented\n";
}
void SymbolTableGenerator::visitParameterExpression(
    const ast::Parameter &value) {
	std::cerr << "visitParameterExpression not implemented\n";
}

} // namespace ray::compiler::analyzer::symbols
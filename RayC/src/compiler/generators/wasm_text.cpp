#include <ray/compiler/generators/wasm_text.hpp>

namespace ray::compiler::generator {

// Statement
void WASMTextGenerator::visitBlockStatement(const ast::Block &value) {}
void WASMTextGenerator::visitTerminalExprStatement(
    const ast::TerminalExpr &value) {}
void WASMTextGenerator::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {}
void WASMTextGenerator::visitFunctionStatement(const ast::Function &value) {}
void WASMTextGenerator::visitIfStatement(const ast::If &value) {}
void WASMTextGenerator::visitJumpStatement(const ast::Jump &value) {}
void WASMTextGenerator::visitVarStatement(const ast::Var &value) {}
void WASMTextGenerator::visitWhileStatement(const ast::While &value) {}
// Expression
void WASMTextGenerator::visitAssignExpression(const ast::Assign &value) {}
void WASMTextGenerator::visitBinaryExpression(const ast::Binary &value) {}
void WASMTextGenerator::visitCallExpression(const ast::Call &value) {}
void WASMTextGenerator::visitGetExpression(const ast::Get &value) {}
void WASMTextGenerator::visitGroupingExpression(const ast::Grouping &value) {}
void WASMTextGenerator::visitLiteralExpression(const ast::Literal &value) {}
void WASMTextGenerator::visitLogicalExpression(const ast::Logical &value) {}
void WASMTextGenerator::visitSetExpression(const ast::Set &value) {}
void WASMTextGenerator::visitUnaryExpression(const ast::Unary &value) {}
void WASMTextGenerator::visitVariableExpression(const ast::Variable &value) {}
void WASMTextGenerator::visitTypeExpression(const ast::Type &value) {}
void WASMTextGenerator::visitParameterExpression(const ast::Parameter &value) {}

bool WASMTextGenerator::hasFailed() const { return false; }

std::string WASMTextGenerator::getOutput() const { return output.str(); }
} // namespace ray::compiler::generator

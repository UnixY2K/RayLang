#pragma once
#include <cstddef>
#include <sstream>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>

namespace ray::compiler::generator {

class WASMTextGenerator : public ast::StatementVisitor,
                          public ast::ExpressionVisitor {
	std::stringstream output;
	size_t ident = 0;

	std::string currentIdent() const;

  public:
	void resolve(const std::vector<std::unique_ptr<ast::Statement>> &statement);

	bool hasFailed() const;

	std::string getOutput() const;

	// Statement
	void visitBlockStatement(const ast::Block &value) override;
	void visitTerminalExprStatement(const ast::TerminalExpr &value) override;
	void
	visitExpressionStmtStatement(const ast::ExpressionStmt &value) override;
	void visitFunctionStatement(const ast::Function &value) override;
	void visitIfStatement(const ast::If &value) override;
	void visitJumpStatement(const ast::Jump &value) override;
	void visitVarStatement(const ast::Var &value) override;
	void visitWhileStatement(const ast::While &value) override;
	void visitStructStatement(const ast::Struct &value) override;
	void visitNamespaceStatement(const ast::Namespace& value) override;
	// Expression
	void visitAssignExpression(const ast::Assign &value) override;
	void visitBinaryExpression(const ast::Binary &value) override;
	void visitCallExpression(const ast::Call &value) override;
	void visitGetExpression(const ast::Get &value) override;
	void visitGroupingExpression(const ast::Grouping &value) override;
	void visitLiteralExpression(const ast::Literal &value) override;
	void visitLogicalExpression(const ast::Logical &value) override;
	void visitSetExpression(const ast::Set &value) override;
	void visitUnaryExpression(const ast::Unary &value) override;
	void visitArrayAccessExpression(const ast::ArrayAccess &value) override;
	void visitVariableExpression(const ast::Variable &value) override;
	void visitTypeExpression(const ast::Type &value) override;
	void visitCastExpression(const ast::Cast &value) override;
	void visitParameterExpression(const ast::Parameter &value) override;
};

} // namespace ray::compiler::generator

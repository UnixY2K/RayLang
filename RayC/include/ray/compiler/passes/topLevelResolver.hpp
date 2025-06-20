#pragma once
#include <memory>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>

namespace ray::compiler::analyzer::symbols {
struct Symbol {
	enum class SymbolType { Function, Struct };
	std::string name;
	std::string mangledName;
	SymbolType type;
	std::string scope;
	const ast::Statement *object;
};
using SymbolTable = std::vector<Symbol>;
class TopLevelResolver : public ast::StatementVisitor,
                         public ast::ExpressionVisitor {
	std::vector<std::string_view> namespaceStack;
	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;

	SymbolTable globalTable;
	size_t top = 0;

  public:
	void resolve(const std::vector<std::unique_ptr<ast::Statement>> &statement);

	SymbolTable getSymbolTable() const;

	bool hasFailed() const;

  private:
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
	void visitNamespaceStatement(const ast::Namespace &value) override;
	void visitCompDirectiveStatement(const ast::CompDirective &value) override;
	void visitImportStatement(const ast::Import& value) override;
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
	void visitIntrinsicExpression(const ast::Intrinsic &value) override;
	void visitTypeExpression(const ast::Type &value) override;
	void visitCastExpression(const ast::Cast &value) override;
	void visitParameterExpression(const ast::Parameter &value) override;
};
} // namespace ray::compiler::analyzer::symbols

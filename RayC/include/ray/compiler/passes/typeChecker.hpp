#pragma once
#include <functional>
#include <memory>
#include <optional>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/message_bag.hpp>

namespace ray::compiler::analyzer {

class TypeChecker : public ast::StatementVisitor,
                    public ast::ExpressionVisitor {
	MessageBag messageBag;

	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t top = 0;

	lang::S1SourceUnit s1SourceUnit;
	std::vector<lang::Type> typeStack;

	lang::SourceUnit currentSourceUnit;
	std::reference_wrapper<lang::Scope> currentScope;
	lang::ModuleStore &moduleStore;

  public:
	TypeChecker(std::string filePath, lang::S1SourceUnit s1SourceUnit,
	            lang::ModuleStore &moduleStore)
	    : messageBag(filePath), s1SourceUnit(s1SourceUnit), typeStack(),
	      currentSourceUnit(), currentScope(currentSourceUnit.scopes[0]),
	      moduleStore(moduleStore) {}

	void resolve(const std::vector<std::unique_ptr<ast::Statement>> &statement);

	const lang::SourceUnit &getCurrentSourceUnit() { return currentSourceUnit; }

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;
	const std::vector<std::string> getWarnings() const;

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
	void visitCompDirectiveStatement(const ast::CompDirective &value) override;
	// Expression
	void visitVariableExpression(const ast::Variable &value) override;
	void visitIntrinsicExpression(const ast::Intrinsic &value) override;
	void visitAssignExpression(const ast::Assign &value) override;
	void visitBinaryExpression(const ast::Binary &value) override;
	void visitCallExpression(const ast::Call &value) override;
	void visitIntrinsicCallExpression(const ast::IntrinsicCall &value) override;
	void visitGetExpression(const ast::Get &value) override;
	void visitGroupingExpression(const ast::Grouping &value) override;
	void visitLiteralExpression(const ast::Literal &value) override;
	void visitLogicalExpression(const ast::Logical &value) override;
	void visitSetExpression(const ast::Set &value) override;
	void visitUnaryExpression(const ast::Unary &value) override;
	void visitArrayAccessExpression(const ast::ArrayAccess &value) override;
	void visitTypeExpression(const ast::Type &value) override;
	void visitCastExpression(const ast::Cast &value) override;
	void visitParameterExpression(const ast::Parameter &value) override;

	std::optional<lang::Type> resolveType(const ast::Statement &statement);
	std::optional<lang::Type> resolveType(const ast::Expression &expression);
	std::vector<lang::Type> resolveTypes(const ast::Statement &statement);
	std::vector<lang::Type> resolveTypes(const ast::Expression &expression);

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type> findTypeInfo(const std::string_view lexeme);

	lang::Type makePointerType(const lang::Type &innerType);
};
} // namespace ray::compiler::analyzer

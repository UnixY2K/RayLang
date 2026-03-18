#pragma once
#include <functional>
#include <memory>
#include <optional>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/depSourceUnit.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/message_bag.hpp>

namespace ray::compiler::passes {

class TypeChecker : public ast::StatementVisitor,
                    public ast::ExpressionVisitor {
	MessageBag messageBag;

	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t directivesStackTop = 0;

	std::vector<lang::Type> typeStack;

	lang::SourceUnit currentSourceUnit;
	lang::DepSourceUnit depCurrentSourceUnit;
	std::reference_wrapper<lang::Scope> currentScope;
	std::reference_wrapper<const environment::DataModel> currentDataModel;
	// lang::ModuleStore &moduleStore;

  public:
	TypeChecker(std::string filePath, const lang::ModuleStore &moduleStore,
	            const environment::DataModel &dataModel,
	            const lang::SourceUnit &sourceUnit)
	    : messageBag("TYPE-CHECKER", filePath), typeStack(),
	      currentSourceUnit(sourceUnit), depCurrentSourceUnit(),
	      currentScope(currentSourceUnit.rootScope), currentDataModel(dataModel)
	//,moduleStore(moduleStore)
	{}

	void resolve(const std::vector<std::unique_ptr<ast::Statement>> &statement);

	const lang::SourceUnit &getCurrentSourceUnit() const {
		return currentSourceUnit;
	}

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
	void visitVarDeclStatement(const ast::VarDecl &value) override;
	void visitMemberStatement(const ast::Member &value) override;
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
	void visitArrayTypeExpression(const ast::ArrayType &value) override;
	void visitPointerTypeExpression(const ast::PointerType &value) override;
	void visitNamedTypeExpression(const ast::NamedType &value) override;
	void visitCastExpression(const ast::Cast &value) override;
	void visitParameterExpression(const ast::Parameter &value) override;

	std::optional<lang::Type> resolveType(const ast::Statement &statement);
	std::optional<lang::Type> resolveType(const ast::Expression &expression);
	std::vector<lang::Type> resolveTypes(const ast::Statement &statement);
	std::vector<lang::Type> resolveTypes(const ast::Expression &expression);

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type> findTypeInfo(const std::string_view lexeme);

	std::optional<lang::FunctionDeclaration>
	resolveFunctionDeclaration(const ast::Function &functionExpr);

	// gets the current scope
	lang::Scope &getCurrentScope();
	// makes a new child scope and sets it as the root scope
	lang::Scope &makeChildScope();
	// pops until found the passed scope, if not found makes an error
	bool popScope(lang::Scope &scope);
};
} // namespace ray::compiler::passes

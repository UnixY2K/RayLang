#pragma once
#include <functional>
#include <memory>
#include <optional>

#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/syntax/ast/Expression.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>

namespace ray::compiler::passes {

class TypeChecker : public syntax::ast::StatementVisitor,
                    public syntax::ast::ExpressionVisitor {
	MessageBag messageBag;

	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t directivesStackTop = 0;

	std::vector<lang::Type> typeStack;

	lang::SourceUnit currentSourceUnit;
	std::reference_wrapper<lang::Scope> currentScope;
	std::reference_wrapper<const environment::DataModel> currentDataModel;
	// lang::ModuleStore &moduleStore;

  public:
	TypeChecker(std::string filePath, const lang::ModuleStore &moduleStore,
	            const environment::DataModel &dataModel,
	            const lang::SourceUnit &sourceUnit)
	    : messageBag("TYPE-CHECKER", filePath), typeStack(),
	      currentSourceUnit(sourceUnit),
	      currentScope(currentSourceUnit.rootScope), currentDataModel(dataModel)
	//,moduleStore(moduleStore)
	{}

	void resolve(
	    const std::vector<std::unique_ptr<syntax::ast::Statement>> &statement);

	const lang::SourceUnit &getCurrentSourceUnit() const {
		return currentSourceUnit;
	}

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;
	const std::vector<std::string> getWarnings() const;

  private:
	void visitBlockStatement(const syntax::ast::Block &value) override;
	void
	visitTerminalExprStatement(const syntax::ast::TerminalExpr &value) override;
	void visitExpressionStmtStatement(
	    const syntax::ast::ExpressionStmt &value) override;
	void visitFunctionStatement(const syntax::ast::Function &value) override;
	void visitMethodStatement(const syntax::ast::Method &value) override;
	void visitIfStatement(const syntax::ast::If &value) override;
	void visitJumpStatement(const syntax::ast::Jump &value) override;
	void visitVarDeclStatement(const syntax::ast::VarDecl &value) override;
	void visitMemberStatement(const syntax::ast::Member &value) override;
	void visitWhileStatement(const syntax::ast::While &value) override;
	void visitStructStatement(const syntax::ast::Struct &value) override;
	void visitTraitStatement(const syntax::ast::Trait &value) override;
	void visitCompDirectiveStatement(
	    const syntax::ast::CompDirective &value) override;
	// Expression
	void visitVariableExpression(const syntax::ast::Variable &value) override;
	void visitIntrinsicExpression(const syntax::ast::Intrinsic &value) override;
	void visitAssignExpression(const syntax::ast::Assign &value) override;
	void visitBinaryExpression(const syntax::ast::Binary &value) override;
	void visitCallExpression(const syntax::ast::Call &value) override;
	void visitIntrinsicCallExpression(
	    const syntax::ast::IntrinsicCall &value) override;
	void visitGetExpression(const syntax::ast::Get &value) override;
	void visitGroupingExpression(const syntax::ast::Grouping &value) override;
	void visitLiteralExpression(const syntax::ast::Literal &value) override;
	void visitLogicalExpression(const syntax::ast::Logical &value) override;
	void visitSetExpression(const syntax::ast::Set &value) override;
	void visitUnaryExpression(const syntax::ast::Unary &value) override;
	void
	visitArrayAccessExpression(const syntax::ast::ArrayAccess &value) override;
	void visitArrayTypeExpression(const syntax::ast::ArrayType &value) override;
	void visitTupleTypeExpression(const syntax::ast::TupleType &value) override;
	void
	visitPointerTypeExpression(const syntax::ast::PointerType &value) override;
	void visitNamedTypeExpression(const syntax::ast::NamedType &value) override;
	void visitCastExpression(const syntax::ast::Cast &value) override;
	void visitParameterExpression(const syntax::ast::Parameter &value) override;

	std::optional<lang::Type>
	resolveType(const syntax::ast::Statement &statement);
	std::optional<lang::Type>
	resolveType(const syntax::ast::Expression &expression);
	std::vector<lang::Type>
	resolveTypes(const syntax::ast::Statement &statement);
	std::vector<lang::Type>
	resolveTypes(const syntax::ast::Expression &expression);

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type> findTypeInfo(const std::string_view lexeme);

	std::optional<lang::FunctionDeclaration>
	resolveFunctionDeclaration(const syntax::ast::Function &functionExpr);

	// gets the current scope
	lang::Scope &getCurrentScope();
	// makes a new child scope and sets it as the root scope
	lang::Scope &makeChildScope();
	// pops until found the passed scope, if not found makes an error
	bool popScope(lang::Scope &scope);
};
} // namespace ray::compiler::passes

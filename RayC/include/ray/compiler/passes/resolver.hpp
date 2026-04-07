#pragma once

#include <memory>
#include <vector>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/ast/typed/TypedExpression.hpp>
#include <ray/compiler/ast/typed/TypedStatement.hpp>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/passes/typeScanner.hpp>

namespace ray::compiler::passes {
class Resolver : public ast::StatementVisitor, public ast::ExpressionVisitor {

	MessageBag messageBag;

	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t directivesStackTop = 0;

	std::vector<lang::Type> typeStack;
	std::vector<lang::StructMember> structMemberStack;
	std::vector<lang::Method> traitMethodStack;

	std::reference_wrapper<const environment::DataModel> currentDataModel;

	lang::SourceUnit currentSourceUnit;
	lang::ModuleStore &currentModuleStore;
	std::reference_wrapper<lang::Scope> currentScope;

  public:
	Resolver(std::string filePath, const environment::DataModel &dataModel,
	         lang::ModuleStore &moduleStore)
	    : messageBag("RESOLVER", filePath), directivesStack(),
	      currentDataModel(dataModel), currentSourceUnit(),
	      currentModuleStore(moduleStore),
	      currentScope(currentSourceUnit.rootScope) {}

	void
	resolve(const std::vector<std::unique_ptr<ast::Statement>> &statements);

	const lang::SourceUnit &getCurrentSourceUnit() const {
		return currentSourceUnit;
	}

	bool hasFailed() const;
	const MessageBag &getMessageBag() const;

  private:
	// Statement
	void visitBlockStatement(const ast::Block &value) override;
	void visitTerminalExprStatement(const ast::TerminalExpr &value) override;
	void
	visitExpressionStmtStatement(const ast::ExpressionStmt &value) override;
	void visitFunctionStatement(const ast::Function &value) override;
	void visitMethodStatement(const ast::Method &value) override;
	void visitIfStatement(const ast::If &value) override;
	void visitJumpStatement(const ast::Jump &value) override;
	void visitVarDeclStatement(const ast::VarDecl &value) override;
	void visitMemberStatement(const ast::Member &value) override;
	void visitWhileStatement(const ast::While &value) override;
	void visitStructStatement(const ast::Struct &value) override;
	void visitTraitStatement(const ast::Trait &value) override;
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
	void visitTupleTypeExpression(const ast::TupleType &value) override;
	void visitPointerTypeExpression(const ast::PointerType &value) override;
	void visitNamedTypeExpression(const ast::NamedType &value) override;
	void visitCastExpression(const ast::Cast &value) override;
	void visitParameterExpression(const ast::Parameter &value) override;

	lang::Type resolveType(const ast::Statement &statement);
	lang::Type resolveType(const ast::Expression &expression);
	std::vector<lang::Type> resolveTypes(const ast::Statement &statement);
	std::vector<lang::Type> resolveTypes(const ast::Expression &expression);
	// only used when you do not care about returned types but
	// want to traverse the items to perform checks and discovery of types
	void discardTypes(const ast::Statement &statement);
	void discardTypes(const ast::Expression &expression);

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	lang::Type findTypeInfo(const std::string_view lexeme);

	std::optional<lang::FunctionDeclaration>
	resolveFunctionDeclaration(const ast::Function &functionExpr);

	// gets the current scope
	lang::Scope &getCurrentScope();
	// makes a new child scope and sets it as the root scope
	lang::Scope &makeChildScope();
	// pops until located at the requested scope, if not found makes an error
	bool popScope(lang::Scope &scope);
};

} // namespace ray::compiler::passes
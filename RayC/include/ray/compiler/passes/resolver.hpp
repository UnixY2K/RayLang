#pragma once

#include <memory>
#include <vector>

#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/passes/typeScanner.hpp>
#include <ray/compiler/syntax/ast/Expression.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>
#include <ray/compiler/syntax/rst/Expression.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

namespace ray::compiler::passes {
class Resolver : public syntax::ast::StatementVisitor,
                 public syntax::ast::ExpressionVisitor {

	MessageBag messageBag;

	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t directivesStackTop = 0;

	std::vector<lang::StructMember> structMemberStack;
	std::vector<lang::Method> traitMethodStack;

	std::reference_wrapper<const environment::DataModel> currentDataModel;

	lang::SourceUnit currentSourceUnit;
	lang::ModuleStore &currentModuleStore;
	std::reference_wrapper<lang::Scope> currentScope;

	syntax::rst::Block rootBlock;

	std::vector<std::unique_ptr<syntax::rst::Statement>> statementStack;
	std::vector<std::unique_ptr<syntax::rst::Expression>> expressionStack;

  public:
	Resolver(std::string filePath, const environment::DataModel &dataModel,
	         lang::ModuleStore &moduleStore)
	    : messageBag("RESOLVER", filePath), directivesStack(),
	      currentDataModel(dataModel), currentSourceUnit(),
	      currentModuleStore(moduleStore),
	      currentScope(currentSourceUnit.rootScope),
	      rootBlock({}, Token::makeEOFToken()) {}

	void resolve(
	    const std::vector<std::unique_ptr<syntax::ast::Statement>> &statements);

	const lang::SourceUnit &getCurrentSourceUnit() const {
		return currentSourceUnit;
	}

	bool hasFailed() const;
	const MessageBag &getMessageBag() const;

  private:
	// Statement
	void visitBlockStatement(const syntax::ast::Block &value) override;
	void
	visitTerminalExprStatement(const syntax::ast::TerminalExpr &value) override;
	void visitExpressionStmtStatement(
	    const syntax::ast::ExpressionStmt &value) override;
	void visitFunctionStatement(const syntax::ast::Function &value) override;
	void
	visitTraitMethodStatement(const syntax::ast::TraitMethod &value) override;
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

	std::unique_ptr<syntax::rst::Statement>
	resolveStatement(const syntax::ast::Statement &statement);
	std::vector<std::unique_ptr<syntax::rst::Statement>>
	resolveStatements(const syntax::ast::Statement &statement);

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	lang::Type findTypeInfo(const std::string_view lexeme);

	std::optional<lang::FunctionDeclaration>
	resolveFunctionDeclaration(const syntax::ast::Function &functionExpr);

	std::vector<std::unique_ptr<directive::CompilerDirective>>
	collectCompilerDirectives();

	// gets the current scope
	lang::Scope &getCurrentScope();
	// makes a new child scope and sets it as the root scope
	lang::Scope &makeChildScope();
	// pops until located at the requested scope, if not found makes an error
	bool popScope(lang::Scope &scope);
};

} // namespace ray::compiler::passes

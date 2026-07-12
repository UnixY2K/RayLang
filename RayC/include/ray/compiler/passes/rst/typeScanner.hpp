#pragma once
#include <cstddef>

#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/trait.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/syntax/rst/Expression.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

namespace ray::compiler::passes {
class TypeScanner : public syntax::rst::StatementVisitor,
                    public syntax::rst::ExpressionVisitor {
	MessageBag messageBag;

	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t directivesStackTop = 0;

	std::vector<lang::Type> typeStack;
	std::vector<lang::StructMember> structMemberStack;
	std::vector<lang::Method> traitMethodStack;

	std::reference_wrapper<const environment::DataModel> currentDataModel;

	lang::SourceUnit &currentSourceUnit;
	lang::ModuleStore &currentModuleStore;
	std::reference_wrapper<lang::Scope> currentScope;

  public:
	TypeScanner(std::string filePath, const environment::DataModel &dataModel,
	            lang::SourceUnit &sourceUnit, lang::ModuleStore &moduleStore)
	    : messageBag("TYPE-SCANNER", filePath), directivesStack(),
	      currentDataModel(dataModel), currentSourceUnit(sourceUnit),
	      currentModuleStore(moduleStore),
	      currentScope(currentSourceUnit.rootScope) {}

	void resolve(const syntax::rst::Block &statement);

	const lang::SourceUnit &getCurrentSourceUnit() const {
		return currentSourceUnit;
	}

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;
	const std::vector<std::string> getWarnings() const;

  private:
	void visitBlockStatement(const syntax::rst::Block &value) override;
	void visitTerminalExpressionStatement(
	    const syntax::rst::TerminalExpression &value) override;
	void visitExpressionStatementStatement(
	    const syntax::rst::ExpressionStatement &value) override;
	void visitFunctionStatement(const syntax::rst::Function &value) override;
	void visitIfStatement(const syntax::rst::If &value) override;
	void visitJumpStatement(const syntax::rst::Jump &value) override;
	void visitVarDeclStatement(const syntax::rst::VarDecl &value) override;
	void visitMemberStatement(const syntax::rst::Member &value) override;
	void visitWhileStatement(const syntax::rst::While &value) override;
	void visitStructStatement(const syntax::rst::Struct &value) override;
	void
	visitPlaceholderStatement(const syntax::rst::Placeholder &value) override;
	// Expression
	void visitVariableExpression(const syntax::rst::Variable &value) override;
	void visitIntrinsicExpression(const syntax::rst::Intrinsic &value) override;
	void visitAssignExpression(const syntax::rst::Assign &value) override;
	void visitBinaryExpression(const syntax::rst::Binary &value) override;
	void visitCallExpression(const syntax::rst::Call &value) override;
	void visitIntrinsicCallExpression(
	    const syntax::rst::IntrinsicCall &value) override;
	void visitGetExpression(const syntax::rst::Get &value) override;
	void visitGroupingExpression(const syntax::rst::Grouping &value) override;
	void visitLiteralExpression(const syntax::rst::Literal &value) override;
	void visitLogicalExpression(const syntax::rst::Logical &value) override;
	void visitSetExpression(const syntax::rst::Set &value) override;
	void visitUnaryExpression(const syntax::rst::Unary &value) override;
	void
	visitArrayAccessExpression(const syntax::rst::ArrayAccess &value) override;
	void visitArrayTypeExpression(const syntax::rst::ArrayType &value) override;
	void visitTupleTypeExpression(const syntax::rst::TupleType &value) override;
	void
	visitPointerTypeExpression(const syntax::rst::PointerType &value) override;
	void visitNamedTypeExpression(const syntax::rst::NamedType &value) override;
	void visitCastExpression(const syntax::rst::Cast &value) override;
	void visitParameterExpression(const syntax::rst::Parameter &value) override;
	void
	visitPlaceHolderExpression(const syntax::rst::PlaceHolder &value) override;

	lang::Type resolveType(const syntax::rst::Statement &statement);
	lang::Type resolveType(const syntax::rst::Expression &expression);
	std::vector<lang::Type>
	resolveTypes(const syntax::rst::Statement &statement);
	std::vector<lang::Type>
	resolveTypes(const syntax::rst::Expression &expression);
	// only used when you do not care about returned types but
	// want to traverse the items to perform checks and discovery of types
	void discardTypes(const syntax::rst::Statement &statement);
	void discardTypes(const syntax::rst::Expression &expression);

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	lang::Type findTypeInfo(const std::string_view lexeme);

	std::optional<lang::FunctionDeclaration>
	resolveFunctionDeclaration(const syntax::rst::Function &functionExpr);

	// gets the current scope
	lang::Scope &getCurrentScope();
	// makes a new child scope and sets it as the root scope
	lang::Scope &makeChildScope();
	// pops until located at the requested scope, if not found makes an error
	bool returnScope(lang::Scope &scope);

	void discoverStruct(const syntax::rst::Struct &structAst);
};
} // namespace ray::compiler::passes
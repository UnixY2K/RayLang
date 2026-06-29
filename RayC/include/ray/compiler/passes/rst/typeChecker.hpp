#pragma once
#include <optional>

#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/moduleStore.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/syntax/rst/Expression.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

namespace ray::compiler::passes::rst {
class TypeChecker : public syntax::rst::StatementVisitor,
                    syntax::rst::ExpressionVisitor {
	MessageBag messageBag;

	std::vector<lang::Type> typeStack;

	lang::SourceUnit &currentSourceUnit;
	std::reference_wrapper<lang::Scope> currentScope;
	std::reference_wrapper<const environment::DataModel> currentDataModel;

	syntax::rst::Block rootBlock;

  public:
	TypeChecker(std::string filePath, const lang::ModuleStore &moduleStore,
	            const environment::DataModel &dataModel,
	            lang::SourceUnit &sourceUnit)
	    : messageBag("TYPE-CHECKER", filePath), typeStack(),
	      currentSourceUnit(sourceUnit),
	      currentScope(currentSourceUnit.rootScope),
	      currentDataModel(dataModel), rootBlock({}, Token::makeEOFToken())
	//,moduleStore(moduleStore)
	{}

	void resolve(syntax::rst::Block &rootBlock);

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

	std::optional<lang::Type>
	resolveType(const syntax::rst::Statement &statement);
	std::optional<lang::Type>
	resolveType(const syntax::rst::Expression &expression);
	std::vector<lang::Type>
	resolveTypes(const syntax::rst::Statement &statement);
	std::vector<lang::Type>
	resolveTypes(const syntax::rst::Expression &expression);

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type> findTypeInfo(const std::string_view lexeme);

	std::optional<lang::FunctionDeclaration>
	resolveFunctionDeclaration(const syntax::rst::Function &functionRST);

	// gets the current scope
	lang::Scope &getCurrentScope();
	// makes a new child scope and sets it as the root scope
	lang::Scope &makeChildScope();
	// pops until found the passed scope, if not found makes an error
	bool popScope(lang::Scope &scope);
};
} // namespace ray::compiler::passes::rst
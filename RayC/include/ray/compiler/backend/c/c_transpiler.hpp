#pragma once

#include <cstddef>
#include <functional>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <vector>

#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/syntax/rst/Expression.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

namespace ray::compiler::backend::c {

class CTranspilerGenerator : public syntax::rst::StatementVisitor,
                             public syntax::rst::ExpressionVisitor {
	MessageBag messageBag;
	std::stringstream output;
	size_t ident = 0;

	std::string currentIdent() const;

	passes::mangling::NameMangler nameMangler;

	std::reference_wrapper<const lang::SourceUnit> currentSourceUnit;
	std::reference_wrapper<const lang::Scope> currentScope;

	std::reference_wrapper<const environment::DataModel> currentDataModel;

  public:
	CTranspilerGenerator(std::string filePath,
	                     const lang::SourceUnit &sourceUnit,
	                     const environment::DataModel &dataModel);

	void resolve(const syntax::rst::Block &statement);

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;

	std::string getOutput() const;

	// Statement
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

  private:
	void transpile(const syntax::rst::Statement &statementAST);
	void transpile(const syntax::rst::Expression &expressionAST);

	void visitType(const lang::Type &type);

	std::string findCallableName(const syntax::rst::Call &callable,
	                             const std::string_view name) const;
	std::string findStructName(const std::string_view name) const;

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type> findTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type>
	getTypeExpression(const syntax::rst::Expression *);

	void defineStruct(std::unordered_set<size_t> &visitedStructs,
	                  const lang::Struct &);
};

} // namespace ray::compiler::backend::c

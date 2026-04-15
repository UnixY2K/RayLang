#pragma once

#include <cstddef>
#include <functional>
#include <memory>
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
#include <ray/compiler/syntax/ast/Expression.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>

namespace ray::compiler::generator::c {

class CTranspilerGenerator : public syntax::ast::StatementVisitor,
                             public syntax::ast::ExpressionVisitor {
	MessageBag messageBag;
	std::stringstream output;
	size_t ident = 0;

	std::string currentIdent() const;

	std::vector<std::string_view> namespaceStack;
	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t top = 0;

	passes::mangling::NameMangler nameMangler;

	std::reference_wrapper<const lang::SourceUnit> currentSourceUnit;
	std::reference_wrapper<const lang::Scope> currentScope;

	std::reference_wrapper<const environment::DataModel> currentDataModel;

  public:
	CTranspilerGenerator(std::string filePath,
	                     const lang::SourceUnit &sourceUnit,
	                     const environment::DataModel &dataModel);

	void resolve(const std::vector<std::unique_ptr<syntax::ast::Statement>> &statement);

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;

	std::string getOutput() const;

	// Statement
	void visitBlockStatement(const syntax::ast::Block &value) override;
	void visitTerminalExprStatement(const syntax::ast::TerminalExpr &value) override;
	void
	visitExpressionStmtStatement(const syntax::ast::ExpressionStmt &value) override;
	void visitFunctionStatement(const syntax::ast::Function &value) override;
	void visitMethodStatement(const syntax::ast::Method &value) override;
	void visitIfStatement(const syntax::ast::If &value) override;
	void visitJumpStatement(const syntax::ast::Jump &value) override;
	void visitVarDeclStatement(const syntax::ast::VarDecl &value) override;
	void visitMemberStatement(const syntax::ast::Member &value) override;
	void visitWhileStatement(const syntax::ast::While &value) override;
	void visitStructStatement(const syntax::ast::Struct &value) override;
	void visitTraitStatement(const syntax::ast::Trait &value) override;
	void visitCompDirectiveStatement(const syntax::ast::CompDirective &value) override;
	// Expression
	void visitVariableExpression(const syntax::ast::Variable &value) override;
	void visitIntrinsicExpression(const syntax::ast::Intrinsic &value) override;
	void visitAssignExpression(const syntax::ast::Assign &value) override;
	void visitBinaryExpression(const syntax::ast::Binary &value) override;
	void visitCallExpression(const syntax::ast::Call &value) override;
	void visitIntrinsicCallExpression(const syntax::ast::IntrinsicCall &value) override;
	void visitGetExpression(const syntax::ast::Get &value) override;
	void visitGroupingExpression(const syntax::ast::Grouping &value) override;
	void visitLiteralExpression(const syntax::ast::Literal &value) override;
	void visitLogicalExpression(const syntax::ast::Logical &value) override;
	void visitSetExpression(const syntax::ast::Set &value) override;
	void visitUnaryExpression(const syntax::ast::Unary &value) override;
	void visitArrayAccessExpression(const syntax::ast::ArrayAccess &value) override;
	void visitArrayTypeExpression(const syntax::ast::ArrayType &value) override;
	void visitTupleTypeExpression(const syntax::ast::TupleType &value) override;
	void visitPointerTypeExpression(const syntax::ast::PointerType &value) override;
	void visitNamedTypeExpression(const syntax::ast::NamedType &value) override;
	void visitCastExpression(const syntax::ast::Cast &value) override;
	void visitParameterExpression(const syntax::ast::Parameter &value) override;

  private:
	void visitType(const lang::Type &type);

	std::string findCallableName(const syntax::ast::Call &callable,
	                             const std::string_view name) const;
	std::string findStructName(const std::string_view name) const;

	std::optional<lang::Type> findScalarTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type> findTypeInfo(const std::string_view lexeme);
	std::optional<lang::Type> getTypeExpression(const syntax::ast::Expression *);

	void defineStruct(std::unordered_set<size_t> &visitedStructs,
	                  const lang::Struct &);
};

} // namespace ray::compiler::generator::c

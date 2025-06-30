#pragma once

#include <cstddef>
#include <memory>
#include <sstream>
#include <string_view>
#include <vector>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/topLevelResolver.hpp>
#include <ray/compiler/types/types.hpp>

namespace ray::compiler::generator::c {

struct Symbol {
	enum class SymbolType { Function, Struct };
	std::string name;
	std::string mangledName;
	SymbolType type;
	std::string scope;
	const ast::Statement *object;
};
using SymbolTable = std::vector<Symbol>;

class CTranspilerGenerator : public ast::StatementVisitor,
                             public ast::ExpressionVisitor {
	MessageBag messageBag;
	std::stringstream output;
	size_t ident = 0;

	std::string currentIdent() const;

	std::vector<std::string_view> namespaceStack;
	std::vector<std::unique_ptr<directive::CompilerDirective>> directivesStack;
	size_t top = 0;

	SymbolTable symbolTable;

	passes::mangling::NameMangler nameMangler;

  public:
	CTranspilerGenerator(std::string filePath) : messageBag(filePath) {}

	void resolve(const std::vector<std::unique_ptr<ast::Statement>> &statement,
	             const lang::SourceUnit &sourceUnit);

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;

	std::string getOutput() const;

	// Statement
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

  private:
	std::string findCallableName(const ast::Call &callable,
	                             const std::string_view name) const;
	std::string findStructName(const std::string_view name) const;

	std::optional<types::TypeInfo>
	findScalarTypeInfo(const std::string_view lexeme);
	std::optional<types::TypeInfo> findTypeInfo(const std::string_view lexeme);
	std::optional<types::TypeInfo> getTypeExpression(const ast::Expression *);
};

} // namespace ray::compiler::generator::c

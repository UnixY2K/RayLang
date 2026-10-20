#pragma once

#include <ray/compiler/passes/compilerPass.hpp>

#include <ray/compiler/syntax/ast/Expression.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>

namespace ray::compiler::passes::rst {

class ModuleResolver : public syntax::rst::StatementVisitor,
                       public syntax::rst::ExpressionVisitor,
                       public CompilerPass {
	infrastructure::CompilationContext *compilationContext;
	std::unique_ptr<infrastructure::CompilerArtifact> currentCompilerArtifact;

	lang::Scope *currentScope;

  public:
	std::string_view name() const override { return "moduleResolver"; }
	void run(infrastructure::CompilationContext &ctx,
	         std::unique_ptr<infrastructure::CompilerArtifact>
	             previousCompilerArtifact) override;
	std::unique_ptr<infrastructure::CompilerArtifact>
	getCompilationArtifact() override;

	~ModuleResolver() = default;

  private:
	void resolve(const syntax::rst::Block &statement);
	// statement
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
};

} // namespace ray::compiler::passes::rst
#include <cassert>
#include <format>
#include <ranges>

#include <ray/compiler/passes/rst/moduleResolver.hpp>

namespace ray::compiler::passes::rst {
void ModuleResolver::run(infrastructure::CompilationContext &ctx,
                         std::unique_ptr<infrastructure::CompilerArtifact>
                             previousCompilerArtifact) {
	compilationContext = &ctx;

	currentScope = &compilationContext->sourceUnit.rootScope;
	assert(previousCompilerArtifact->rootRSTBlock.has_value());
	currentCompilerArtifact = std::move(previousCompilerArtifact);
	auto rootRST = currentCompilerArtifact->rootRSTBlock->get();
	resolve(*rootRST);
}
std::unique_ptr<infrastructure::CompilerArtifact>
ModuleResolver::getCompilationArtifact() {
	return std::move(currentCompilerArtifact);
}

void ModuleResolver::resolve(const syntax::rst::Block &block) {
	// iterate each statement, search and then define modules in the respective
	// scope context
	const auto &statements = block.statements;
	for (const auto &statement : statements) {
		statement->visit(*this);
	}
}
// statement
void ModuleResolver::visitBlockStatement(const syntax::rst::Block &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitTerminalExpressionStatement(
    const syntax::rst::TerminalExpression &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitExpressionStatementStatement(
    const syntax::rst::ExpressionStatement &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitFunctionStatement(
    const syntax::rst::Function &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitIfStatement(const syntax::rst::If &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitJumpStatement(const syntax::rst::Jump &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitVarDeclStatement(const syntax::rst::VarDecl &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitMemberStatement(const syntax::rst::Member &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitWhileStatement(const syntax::rst::While &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitStructStatement(const syntax::rst::Struct &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitPlaceholderStatement(
    const syntax::rst::Placeholder &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
// Expression
void ModuleResolver::visitVariableExpression(
    const syntax::rst::Variable &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitIntrinsicExpression(
    const syntax::rst::Intrinsic &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitAssignExpression(const syntax::rst::Assign &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitBinaryExpression(const syntax::rst::Binary &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitCallExpression(const syntax::rst::Call &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitIntrinsicCallExpression(
    const syntax::rst::IntrinsicCall &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitGetExpression(const syntax::rst::Get &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitGroupingExpression(
    const syntax::rst::Grouping &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitLiteralExpression(const syntax::rst::Literal &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitLogicalExpression(const syntax::rst::Logical &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitSetExpression(const syntax::rst::Set &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitUnaryExpression(const syntax::rst::Unary &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitArrayAccessExpression(
    const syntax::rst::ArrayAccess &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitArrayTypeExpression(
    const syntax::rst::ArrayType &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitTupleTypeExpression(
    const syntax::rst::TupleType &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitPointerTypeExpression(
    const syntax::rst::PointerType &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitNamedTypeExpression(
    const syntax::rst::NamedType &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitCastExpression(const syntax::rst::Cast &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitParameterExpression(
    const syntax::rst::Parameter &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void ModuleResolver::visitPlaceHolderExpression(
    const syntax::rst::PlaceHolder &value) {
	compilationContext->diagnostics.error(
	    value.getToken(),
	    std::format("{} not implemented", __PRETTY_FUNCTION__));
}

} // namespace ray::compiler::passes::rst
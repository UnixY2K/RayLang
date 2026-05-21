#include <format>

#include <ray/compiler/passes/rst/typeChecker.hpp>

namespace ray::compiler::passes::rst {

void TypeChecker::resolve(syntax::rst::Block &rootBlock) {}

bool TypeChecker::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TypeChecker::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TypeChecker::getWarnings() const {
	return messageBag.getWarnings();
}

void TypeChecker::visitBlockStatement(const syntax::rst::Block &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitTerminalExpressionStatement(
    const syntax::rst::TerminalExpression &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitExpressionStatementStatement(
    const syntax::rst::ExpressionStatement &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitFunctionStatement(const syntax::rst::Function &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitIfStatement(const syntax::rst::If &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitJumpStatement(const syntax::rst::Jump &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitVarDeclStatement(const syntax::rst::VarDecl &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitMemberStatement(const syntax::rst::Member &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitWhileStatement(const syntax::rst::While &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitStructStatement(const syntax::rst::Struct &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitPlaceholderStatement(
    const syntax::rst::Placeholder &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}

// expression visitor
void TypeChecker::visitVariableExpression(const syntax::rst::Variable &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitIntrinsicExpression(
    const syntax::rst::Intrinsic &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitAssignExpression(const syntax::rst::Assign &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitBinaryExpression(const syntax::rst::Binary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitCallExpression(const syntax::rst::Call &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitIntrinsicCallExpression(
    const syntax::rst::IntrinsicCall &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitGetExpression(const syntax::rst::Get &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitGroupingExpression(const syntax::rst::Grouping &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitLiteralExpression(const syntax::rst::Literal &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitLogicalExpression(const syntax::rst::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitSetExpression(const syntax::rst::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitUnaryExpression(const syntax::rst::Unary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitArrayAccessExpression(
    const syntax::rst::ArrayAccess &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitArrayTypeExpression(
    const syntax::rst::ArrayType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitTupleTypeExpression(
    const syntax::rst::TupleType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitPointerTypeExpression(
    const syntax::rst::PointerType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitNamedTypeExpression(
    const syntax::rst::NamedType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitCastExpression(const syntax::rst::Cast &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitParameterExpression(
    const syntax::rst::Parameter &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitPlaceHolderExpression(
    const syntax::rst::PlaceHolder &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}

} // namespace ray::compiler::passes::rst
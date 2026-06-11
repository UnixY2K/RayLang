#include <format>

#include <ray/compiler/passes/rst/typeChecker.hpp>

namespace ray::compiler::passes::rst {

void TypeChecker::resolve(syntax::rst::Block &rootBlock) {
	rootBlock.visit(*this);
}

bool TypeChecker::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TypeChecker::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TypeChecker::getWarnings() const {
	return messageBag.getWarnings();
}

void TypeChecker::visitBlockStatement(const syntax::rst::Block &blockRST) {

	std::vector<lang::Type> types;
	for (const auto &statement : blockRST.statements) {
		auto stmtTypes = resolveTypes(*statement);
		types.reserve(types.size() + stmtTypes.size());
		for (const auto &type : stmtTypes) {
			if (type != lang::Type::defineStmtType()) {
				types.push_back(type);
			}
		}
	}

	typeStack.reserve(typeStack.size() + types.size());
	typeStack.insert(typeStack.end(), types.begin(), types.end());
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

std::optional<lang::Type>
TypeChecker::resolveType(const syntax::rst::Statement &statement) {
	auto types = resolveTypes(statement);

	if (types.size() > 1) {
		// check wether the return types coerce
		for (size_t i = 1; i < types.size(); i++) {
			if (!types[0].coercercesInto(types[i])) {
				messageBag.bug(
				    statement.getToken(),
				    std::format(
				        "'{}' return types does not match for expected '{}' vs '{}'",
				        statement.variantName(), types[0].name, types[i].name));
			}
		}
	}

	return types.size() > 0 ? std::optional<lang::Type>(types[0])
	                        : std::nullopt;
}
std::optional<lang::Type>
TypeChecker::resolveType(const syntax::rst::Expression &expression) {
	auto types = resolveTypes(expression);

	if (types.size() > 1) {
		messageBag.bug(expression.getToken(),
		               std::format("'{}' yield multiple values",
		                           expression.variantName()));
	}

	return types.size() > 0 ? std::optional<lang::Type>(types[0])
	                        : std::nullopt;
}
std::vector<lang::Type>
TypeChecker::resolveTypes(const syntax::rst::Statement &statement) {
	std::vector<lang::Type> returnTypes;
	size_t tsSize = typeStack.size();
	statement.visit(*this);
	while (typeStack.size() > tsSize) {
		auto returnType = typeStack.back();
		typeStack.pop_back();
		returnTypes.push_back(returnType);
	}
	return returnTypes;
}
std::vector<lang::Type>
TypeChecker::resolveTypes(const syntax::rst::Expression &expression) {
	std::vector<lang::Type> returnTypes;
	size_t tsSize = typeStack.size();
	expression.visit(*this);
	while (typeStack.size() > tsSize) {
		auto returnType = typeStack.back();
		typeStack.pop_back();
		returnTypes.push_back(returnType);
	}
	if (returnTypes.size() < 1) {
		messageBag.bug(expression.getToken(),
		               std::format("'{}' did not resolve a type",
		                           expression.variantName()));
		typeStack.push_back(lang::Type::defineUnknownType());
	}
	return returnTypes;
}

} // namespace ray::compiler::passes::rst
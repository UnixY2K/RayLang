#include <format>

#include <ray/compiler/passes/resolver.hpp>

namespace ray::compiler::passes {

void Resolver::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {

	for (const auto &statement : statements) {
		statement->visit(*this);
	}

	for (auto &directive : directivesStack) {
		messageBag.warning(directive->getToken(),
		                   std::format("unused compiler directive {}",
		                               directive->directiveName()));
	}

	if (typeStack.size() > 0) {
		Token errorToken{Token::TokenType::TOKEN_EOF,
		                 std::string(Token::glyph(Token::TokenType::TOKEN_EOF)),
		                 0, 0};
		messageBag.bug(errorToken, std::format("type stack evaluation error"));
	}
}

bool Resolver::hasFailed() const { return messageBag.failed(); }
const MessageBag &Resolver::getMessageBag() const { return messageBag; }

void Resolver::visitBlockStatement(const ast::Block &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitTerminalExprStatement(const ast::TerminalExpr &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitExpressionStmtStatement(const ast::ExpressionStmt &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitFunctionStatement(const ast::Function &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitMethodStatement(const ast::Method &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitIfStatement(const ast::If &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitJumpStatement(const ast::Jump &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitVarDeclStatement(const ast::VarDecl &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitMemberStatement(const ast::Member &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitWhileStatement(const ast::While &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitStructStatement(const ast::Struct &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitTraitStatement(const ast::Trait &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitCompDirectiveStatement(const ast::CompDirective &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitVariableExpression(const ast::Variable &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitAssignExpression(const ast::Assign &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitBinaryExpression(const ast::Binary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitCallExpression(const ast::Call &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitIntrinsicCallExpression(const ast::IntrinsicCall &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitGetExpression(const ast::Get &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitGroupingExpression(const ast::Grouping &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitLiteralExpression(const ast::Literal &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitLogicalExpression(const ast::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitSetExpression(const ast::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitUnaryExpression(const ast::Unary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitArrayAccessExpression(const ast::ArrayAccess &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitArrayTypeExpression(const ast::ArrayType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitTupleTypeExpression(const ast::TupleType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitPointerTypeExpression(const ast::PointerType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitNamedTypeExpression(const ast::NamedType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitCastExpression(const ast::Cast &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitParameterExpression(const ast::Parameter &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}

lang::Scope &Resolver::getCurrentScope() { return currentScope.get(); }
lang::Scope &Resolver::makeChildScope() {
	currentScope = currentScope.get().makeChildScope();
	return currentScope;
}
bool Resolver::popScope(lang::Scope &targetScope) {
	lang::Scope *scope = &getCurrentScope();
	while (scope != nullptr) {
		if (scope == &targetScope) {
			if (scope->getParentScope().has_value()) {
				currentScope = scope->getParentScope()->get();
			} else {
				currentScope = *scope;
				messageBag.bug(
				    {},
				    "found scope to pop but no parent scope, setting current scope to found scope");
			}
			return true;
		}
		auto scopeRef = scope->getParentScope();
		lang::Scope *parentScope =
		    scopeRef
		        .transform([](std::reference_wrapper<lang::Scope> &scopeRef)
		                       -> lang::Scope * { return &scopeRef.get(); })
		        .value_or(nullptr);
		scope = parentScope;
	}

	messageBag.bug({},
	               "could not pop current scope, pop to first parent scope");
	if (currentScope.get().getParentScope().has_value()) {
		currentScope = currentScope.get().getParentScope().value();
	} else {
		messageBag.bug({},
		               "parent scope not found, setting scope to root scope");
		currentScope = currentSourceUnit.rootScope;
	}
	return false;
}

} // namespace ray::compiler::passes
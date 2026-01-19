#include <format>

#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/typeScanner.hpp>

namespace ray::compiler::passes {

void TypeScanner::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {
	for (const auto &statement : statements) {
		statement->visit(*this);
	}
}

bool TypeScanner::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TypeScanner::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TypeScanner::getWarnings() const {
	return messageBag.getWarnings();
}

void TypeScanner::visitBlockStatement(const ast::Block &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitTerminalExprStatement(const ast::TerminalExpr &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitFunctionStatement(const ast::Function &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitIfStatement(const ast::If &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitJumpStatement(const ast::Jump &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitVarDeclStatement(const ast::VarDecl &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitMemberStatement(const ast::Member &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitWhileStatement(const ast::While &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitStructStatement(const ast::Struct &structObjAst) {
	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > topDirectivesStack; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(),
			    std::format("unmatched compiler directive '{}' for function.\n",
			                directive->directiveName()));
		}
		directivesStack.pop_back();
	}

	std::string structName = std::string(structObjAst.name.getLexeme());
	std::string currentModule;
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(
	        currentModule, structObjAst, linkageDirective);

	auto &scope = makeChildScope();
	// structs can be declared multiple times
	// this is mostly required for C interop
	if (structObjAst.declaration) {
		auto type = currentDataModel.get().declareStructType(structName);
		if (!scope.declareStruct(type, mangledStructName)) {
			messageBag.error(structObjAst.getToken(),
			                 "could not declare struct");
		}
		return;
	}

	if (scope.findStruct(structName)) {
		messageBag.error(
		    structObjAst.getToken(),
		    std::format("{} is defined multiple times", structName));
	}

	auto structRef = currentSourceUnit.declareStruct( //
	    lang::Struct{
	        .name = structName,               //
	        .mangledName = mangledStructName, //
	        .members = {}                     //
	    });
	if (!structRef.has_value()) {
		messageBag.error(structObjAst.getToken(),
		                 std::format("could not bind struct '{}'", structName));
		return;
	}

	scope.bindStruct(structName, structRef.value());
}
void TypeScanner::visitCompDirectiveStatement(const ast::CompDirective &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
// Expression
void TypeScanner::visitVariableExpression(const ast::Variable &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitAssignExpression(const ast::Assign &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitBinaryExpression(const ast::Binary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitCallExpression(const ast::Call &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitIntrinsicCallExpression(
    const ast::IntrinsicCall &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitGetExpression(const ast::Get &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitGroupingExpression(const ast::Grouping &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitLiteralExpression(const ast::Literal &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitLogicalExpression(const ast::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitSetExpression(const ast::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitUnaryExpression(const ast::Unary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitArrayAccessExpression(const ast::ArrayAccess &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitTypeExpression(const ast::Type &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitCastExpression(const ast::Cast &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitParameterExpression(const ast::Parameter &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}

lang::Scope &TypeScanner::getCurrentScope() { return currentScope.get(); }
lang::Scope &TypeScanner::makeChildScope() {
	currentScope = currentScope.get().makeChildScope();
	return currentScope;
}
bool TypeScanner::returnScope(lang::Scope &targetScope) {
	lang::Scope *scope = &getCurrentScope();
	while (scope != nullptr) {
		if (scope == &targetScope) {
			currentScope = *scope;
			return true;
		}
		scope = scope->getParentScope()
		            .transform([](auto v) { return &v.get(); })
		            .value_or(nullptr);
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
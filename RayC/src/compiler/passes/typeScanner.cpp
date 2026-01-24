#include <cassert>
#include <format>
#include <optional>

#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/typeScanner.hpp>
#include <string_view>

namespace ray::compiler::passes {

void TypeScanner::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {
	// search first for structs, then go throught the statements
	for (const auto &statement : statements) {
		if (const auto *structAst =
		        dynamic_cast<const ast::Struct *>(statement.get())) {
			discoverStruct(*structAst);
		}
	}
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
void TypeScanner::visitMemberStatement(const ast::Member &memberAst) {
	std::string memberName = memberAst.name.lexeme;

	memberAst.visit(*this);
	lang::Type memberTypeObj{

	};
	lang::StructMember structMember{
	    // we do not care about this
	    .publicVisibility = false,
	    .isMutable = false,
	    // we care about this
	    .name = memberName,
	    .type = memberTypeObj,
	};

	structMemberStack.push_back(structMember);
}
void TypeScanner::visitWhileStatement(const ast::While &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitStructStatement(const ast::Struct &structAst) {
	if (structAst.declaration) {
		return;
	}

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

	auto structName = structAst.name.getLexeme();
	std::string currentModule;
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structAst,
	                                                 linkageDirective);

	auto &scope = currentScope.get();
	auto structObjRes =
	    scope.findLocalStruct(structName).value_or({}).getObject();
	if (!structObjRes.has_value()) {
		messageBag.bug(structAst.getToken(),
		               std::format("could not find internal reference for {}",
		                           structName));
		return;
	}
	auto structObj = structObjRes.value().get();
	std::vector<lang::StructMember> members;
	for (const auto &member : structAst.members) {
		member.visit(*this);
		assert(!structMemberStack.empty());
		structMemberStack.pop_back();
		auto memberObj = structMemberStack.back();

		members.push_back(memberObj);
	}

	structObj.members = members;
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
void TypeScanner::visitTypeExpression(const ast::Type &typeAst) {
	// TODO: adapt this from type checker 

	
	messageBag.error(typeAst.getToken(),
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

std::optional<lang::Type>
TypeScanner::resolveType(const ast::Statement &statement) {
	auto types = resolveTypes(statement);

	if (types.size() > 1) {
		// check wether the return types coerce
		for (size_t i = 1; i < types.size(); i++) {
			if (!types[0].coercercesInto(types[i])) {
				messageBag.bug(
				    statement.getToken(),
				    std::format(
				        "'{}' return types does not match for '{}' vs '{}'",
				        statement.variantName(), types[0].name, types[i].name));
			}
		}
	}

	return types.size() > 0 ? std::optional<lang::Type>(types[0])
	                        : std::nullopt;
}
std::optional<lang::Type>
TypeScanner::resolveType(const ast::Expression &expression) {
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
TypeScanner::resolveTypes(const ast::Statement &statement) {
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
TypeScanner::resolveTypes(const ast::Expression &expression) {
	std::vector<lang::Type> returnTypes;
	size_t tsSize = typeStack.size();
	expression.visit(*this);
	while (typeStack.size() > tsSize) {
		auto returnType = typeStack.back();
		typeStack.pop_back();
		returnTypes.push_back(returnType);
	}
	if (returnTypes.size() < 1) {
		messageBag.bug(
		    expression.getToken(),
		    std::format("'{}' did not resolve a type, assuming statement",
		                expression.variantName()));
		typeStack.push_back(lang::Type::defineStmtType());
	}
	return returnTypes;
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

void TypeScanner::discoverStruct(const ast::Struct &structAst) {
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

	std::string structName = std::string(structAst.name.getLexeme());
	std::string currentModule;
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structAst,
	                                                 linkageDirective);

	auto &scope = currentScope.get();
	// structs can be declared multiple times
	// this is mostly required for C interop
	if (structAst.declaration) {
		auto type = currentDataModel.get().declareStructType(structName);
		if (!scope.declareStruct(structName)) {
			messageBag.error(structAst.getToken(), "could not declare struct");
		}
		return;
	}

	if (scope.findStruct(structName)) {
		messageBag.error(
		    structAst.getToken(),
		    std::format("{} is defined multiple times", structName));
	}

	if (!currentSourceUnit.bindStruct(
	        lang::Struct{
	            .name = structName,               //
	            .mangledName = mangledStructName, //
	            .members = {}                     //
	        },
	        scope)) {
		messageBag.error(structAst.getToken(),
		                 std::format("could not bind struct '{}'", structName));
		return;
	}
}

} // namespace ray::compiler::passes
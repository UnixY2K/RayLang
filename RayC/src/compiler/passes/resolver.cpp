#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <memory>
#include <optional>

#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/passes/resolver.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/syntax/rst/Expression.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

namespace ray::compiler::passes {

void Resolver::resolve(
    const std::vector<std::unique_ptr<syntax::ast::Statement>> &statements) {

	rootBlock.statements.clear();
	for (const auto &childStatement : statements) {
		auto statement = resolveStatement(*childStatement);
		rootBlock.statements.push_back(std::move(statement));
	}

	for (auto &directive : directivesStack) {
		messageBag.warning(directive->getToken(),
		                   std::format("unused compiler directive {}",
		                               directive->directiveName()));
	}
}

bool Resolver::hasFailed() const { return messageBag.failed(); }
const MessageBag &Resolver::getMessageBag() const { return messageBag; }

void Resolver::visitBlockStatement(const syntax::ast::Block &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitTerminalExprStatement(
    const syntax::ast::TerminalExpr &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitExpressionStmtStatement(
    const syntax::ast::ExpressionStmt &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitFunctionStatement(
    const syntax::ast::Function &functionAST) {

	auto directives = collectCompilerDirectives();
	auto functionRST = std::make_unique<syntax::rst::Function>(
	    syntax::rst::Function(functionAST.name, functionAST.publicVisibility,
	                          {}, std::nullopt, {}, functionAST.token));

	for (const auto &paramAST : functionAST.params) {
		auto paramExpression = resolveExpression(paramAST);
		syntax::rst::Parameter paramRST(
		    paramAST.name, std::move(paramExpression), paramAST.token);
		functionRST->params.push_back(std::move(paramRST));
	}

	auto returnExpression = resolveExpression(*functionAST.returnType);

	if (functionAST.body.has_value()) {
		auto functionBodyRST = resolveStatement(*functionAST.body->get());
		functionRST->body = std::move(functionBodyRST);
	}

	statementStack.push_back(std::move(functionRST));
}
void Resolver::visitTraitMethodStatement(
    const syntax::ast::TraitMethod &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitIfStatement(const syntax::ast::If &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitJumpStatement(const syntax::ast::Jump &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitVarDeclStatement(const syntax::ast::VarDecl &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitMemberStatement(const syntax::ast::Member &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitWhileStatement(const syntax::ast::While &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitStructStatement(const syntax::ast::Struct &structAst) {

	auto compilerDirectives = collectCompilerDirectives();

	std::optional<directive::LinkageDirective> linkageDirective;

	for (const auto &directive : compilerDirectives) {
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(),
			    std::format("unmatched compiler directive '{}' for struct.\n",
			                directive->directiveName()));
		}
	}

	auto structName = structAst.name.getLexeme();
	std::string currentModule;
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structAst,
	                                                 linkageDirective);

	auto &scope = currentScope.get();

	if (!currentSourceUnit.declareStruct(
	        lang::Struct{
	            .opaque = true,                   // unknown implementation
	            .name = std::string(structName),  //
	            .mangledName = mangledStructName, //
	        },
	        scope)) {
		messageBag.error(structAst.getToken(), "could not declare struct");
	}

	std::unique_ptr<syntax::rst::Struct> structRST =
	    std::make_unique<syntax::rst::Struct>(syntax::rst::Struct(
	        structAst.name, structAst.publicVisibility, structAst.declaration,
	        {}, structAst.memberVisibility, structAst.token));

	// don´t bother with declarations
	if (structAst.declaration) {
		statementStack.push_back(std::move(structRST));
		return;
	}

	auto structObjRes = scope.findLocalStruct(structName)
	                        .value_or(util::soft_reference<lang::Struct>())
	                        .getObject();

	if (!structObjRes.has_value()) {
		messageBag.bug(
		    structAst.getToken(),
		    std::format("could not find Struct internal reference for '{}'",
		                structName));
		return;
	}

	for (const auto &member : structAst.members) {
		auto resolvedStatementRST = resolveStatement(member);
		auto memberRSTPtr =
		    dynamic_cast<syntax::rst::Member *>(resolvedStatementRST.get());
		if (!memberRSTPtr) {
			messageBag.bug(
			    member.getToken(),
			    std::format("could not get struct member data for '{}'",
			                member.name.getLexeme()));
			continue;
		}
		// TODO: move the pointer and add it to the membersRST
		structRST->members.push_back(
		    syntax::rst::Member(std::move(*memberRSTPtr)));
	}

	statementStack.push_back(std::move(structRST));
}
void Resolver::visitTraitStatement(const syntax::ast::Trait &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitCompDirectiveStatement(
    const syntax::ast::CompDirective &compDirectiveAst) {
	auto directiveToken = compDirectiveAst.name;
	auto directiveName = compDirectiveAst.name.getLexeme();
	if (directiveName == "Linkage") {
		auto &attributes = compDirectiveAst.values;
		auto directive = directive::LinkageDirective(
		    attributes.find("name") != attributes.end() ? attributes.at("name")
		                                                : "",
		    attributes.find("resolution") != attributes.end()
		        ? attributes.at("resolution") == "external"
		        : false,
		    attributes.find("mangling") != attributes.end()
		        ? attributes.at("mangling") == "c"
		              ? directive::LinkageDirective::ManglingType::C
		              : directive::LinkageDirective::ManglingType::Unknown
		        : directive::LinkageDirective::ManglingType::Default,
		    directiveToken);
		if (!compDirectiveAst.child) {
			messageBag.error(compDirectiveAst.getToken(),
			                 std::format("{} must have a child expression.",
			                             directive.directiveName()));
			return;
		}

		auto childValue = compDirectiveAst.child.get();
		if (!dynamic_cast<syntax::ast::Function *>(childValue) &&
		    !dynamic_cast<syntax::ast::Struct *>(childValue)) {
			messageBag.error(
			    childValue->getToken(),
			    std::format(
			        "{} child expression must be a function or a struct.",
			        directive.directiveName()));
			return;
		}

		size_t startDirectives = directivesStack.size();
		size_t originalTop = directivesStackTop + 1;
		directivesStackTop = startDirectives;
		directivesStack.push_back(
		    std::make_unique<directive::LinkageDirective>(directive));
		auto statementRST = resolveStatement(*compDirectiveAst.child);
		if (directivesStack.size() != startDirectives) {
			messageBag.bug(childValue->getToken(),
			               "unprocessed compiler directives");
		}
		directivesStackTop = originalTop;

		statementStack.push_back(std::move(statementRST));

	} else {
		messageBag.error(
		    compDirectiveAst.getToken(),
		    std::format("Unknown compiler directive '{}'.", directiveName));
	}
}
void Resolver::visitVariableExpression(const syntax::ast::Variable &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitIntrinsicExpression(const syntax::ast::Intrinsic &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitAssignExpression(const syntax::ast::Assign &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitBinaryExpression(const syntax::ast::Binary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitCallExpression(const syntax::ast::Call &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitIntrinsicCallExpression(
    const syntax::ast::IntrinsicCall &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitGetExpression(const syntax::ast::Get &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitGroupingExpression(const syntax::ast::Grouping &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitLiteralExpression(const syntax::ast::Literal &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitLogicalExpression(const syntax::ast::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitSetExpression(const syntax::ast::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitUnaryExpression(const syntax::ast::Unary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitArrayAccessExpression(
    const syntax::ast::ArrayAccess &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitArrayTypeExpression(const syntax::ast::ArrayType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitTupleTypeExpression(const syntax::ast::TupleType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitPointerTypeExpression(
    const syntax::ast::PointerType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitNamedTypeExpression(const syntax::ast::NamedType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitCastExpression(const syntax::ast::Cast &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitParameterExpression(const syntax::ast::Parameter &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}

std::unique_ptr<syntax::rst::Statement>
Resolver::resolveStatement(const syntax::ast::Statement &statementAST) {
	auto statements = resolveStatements(statementAST);

	std::unique_ptr<syntax::rst::Statement> returnStatement;
	if (statements.size() > 1) {
		returnStatement = std::make_unique<syntax::rst::Block>(
		    syntax::rst::Block(std::move(statements), statementAST.getToken()));
	} else if (statements.size() > 0) {
		returnStatement = std::move(statements[0]);
	} else {
		messageBag.bug(
		    statementAST.getToken(),
		    std::format(
		        "'{}' statement did not yield an RST statement, making empty block RST statement",
		        statementAST.variantName()));
		returnStatement = std::make_unique<syntax::rst::Block>(
		    syntax::rst::Block{{}, statementAST.getToken()});
	}

	return returnStatement;
}

std::vector<std::unique_ptr<syntax::rst::Statement>>
Resolver::resolveStatements(const syntax::ast::Statement &statementAST) {
	std::vector<std::unique_ptr<syntax::rst::Statement>> returnStatements;
	size_t tsSize = statementStack.size();
	statementAST.visit(*this);
	while (statementStack.size() > tsSize) {
		auto returnStatement = std::move(statementStack.back());
		statementStack.pop_back();
		returnStatements.push_back(std::move(returnStatement));
	}
	return returnStatements;
}

std::unique_ptr<syntax::rst::Expression>
Resolver::resolveExpression(const syntax::ast::Expression &expressionAST) {
	auto expressions = resolveExpressions(expressionAST);

	std::unique_ptr<syntax::rst::Expression> returnExpression;
	if (expressions.size() > 1) {
		returnExpression =
		    std::make_unique<syntax::rst::TupleType>(syntax::rst::TupleType(
		        false, std::move(expressions), expressionAST.getToken()));
	} else if (expressions.size() > 0) {
		returnExpression = std::move(expressions[0]);
	} else {
		messageBag.bug(
		    expressionAST.getToken(),
		    std::format(
		        "'{}' expression did not yield an RST expression, making empty tuple RST expression",
		        expressionAST.variantName()));
		returnExpression = std::make_unique<syntax::rst::TupleType>(
		    syntax::rst::TupleType{false, {}, expressionAST.getToken()});
	}

	return returnExpression;
}
std::vector<std::unique_ptr<syntax::rst::Expression>>
Resolver::resolveExpressions(const syntax::ast::Expression &expressionAST) {
	std::vector<std::unique_ptr<syntax::rst::Expression>> returnExpressions;
	size_t tsSize = expressionStack.size();
	expressionAST.visit(*this);
	while (expressionStack.size() > tsSize) {
		auto returnExpression = std::move(expressionStack.back());
		expressionStack.pop_back();
		returnExpressions.push_back(std::move(returnExpression));
	}
	return returnExpressions;
}

std::vector<std::unique_ptr<directive::CompilerDirective>>
Resolver::collectCompilerDirectives() {
	std::vector<std::unique_ptr<directive::CompilerDirective>> directives;

	size_t top = std::min(directivesStack.size(), directivesStackTop);

	directives.insert(directives.end(),
	                  std::make_move_iterator(directivesStack.begin() + top),
	                  std::make_move_iterator(directivesStack.end()));

	directivesStack.erase(directivesStack.begin() + top, directivesStack.end());

	directivesStackTop = directivesStack.size();

	return directives;
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

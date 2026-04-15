#include "ray/compiler/lang/type.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <memory>
#include <ranges>

#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
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

	if (!(typeStack | std::views::filter([](const lang::Type &type) {
		      return type.getKind() != lang::TypeKind::abstract;
	      })).empty()) {
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
void Resolver::visitFunctionStatement(const ast::Function &functionAst) {

	auto directives = collectCompilerDirectives();

	// TODO: add to the current generated AST the function and declare it in the
	// current scope so it can be found by the type system at a later step

	messageBag.error(functionAst.getToken(),
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

	auto directives = collectCompilerDirectives();

	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitTraitStatement(const ast::Trait &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void Resolver::visitCompDirectiveStatement(
    const ast::CompDirective &compDirectiveAst) {
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
		if (compDirectiveAst.child) {
			auto childValue = compDirectiveAst.child.get();
			if (dynamic_cast<ast::Function *>(childValue) ||
			    dynamic_cast<ast::Struct *>(childValue)) {
				size_t startDirectives = directivesStack.size();
				size_t originalTop = directivesStackTop + 1;
				directivesStackTop = startDirectives;
				directivesStack.push_back(
				    std::make_unique<directive::LinkageDirective>(directive));
				auto directiveType = resolveType(*compDirectiveAst.child);
				typeStack.push_back(directiveType);

				if (directivesStack.size() != startDirectives) {
					messageBag.bug(childValue->getToken(),
					               "unprocessed compiler directives");
				}

				directivesStackTop = originalTop;
			} else {
				messageBag.error(
				    childValue->getToken(),
				    std::format(
				        "{} child expression must be a function or a struct.",
				        directive.directiveName()));
			}
		} else {
			messageBag.error(compDirectiveAst.getToken(),
			                 std::format("{} must have a child expression.",
			                             directive.directiveName()));
		}
	} else {
		messageBag.error(
		    compDirectiveAst.getToken(),
		    std::format("Unknown compiler directive '{}'.", directiveName));
	}
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

lang::Type Resolver::resolveType(const ast::Statement &statement) {
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

	return types.size() > 0 ? types[0] : lang::Type::defineStmtType();
}
lang::Type Resolver::resolveType(const ast::Expression &expression) {
	auto types = resolveTypes(expression);

	if (types.size() > 1) {
		messageBag.bug(expression.getToken(),
		               std::format("'{}' yield multiple values",
		                           expression.variantName()));
	}

	return types.size() > 0 ? types[0] : lang::Type::defineUnknownType();
}
std::vector<lang::Type>
Resolver::resolveTypes(const ast::Statement &statement) {
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
Resolver::resolveTypes(const ast::Expression &expression) {
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

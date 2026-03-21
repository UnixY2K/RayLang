#include "ray/compiler/ast/expression.hpp"
#include "ray/compiler/lang/functionDefinition.hpp"
#include <cassert>
#include <cstddef>
#include <format>
#include <functional>
#include <optional>
#include <string_view>

#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/typeScanner.hpp>
#include <ray/util/soft_reference.hpp>

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

void TypeScanner::visitBlockStatement(const ast::Block &blockAst) {
	auto &parentScope = currentScope.get();
	currentScope = parentScope.makeChildScope();

	for (const auto &astStatement : blockAst.statements) {
		astStatement->visit(*this);
	}

	currentScope = parentScope;
}
void TypeScanner::visitTerminalExprStatement(
    const ast::TerminalExpr &terminalExprAst) {
	if (terminalExprAst.expression.has_value()) {
		terminalExprAst.expression->get()->visit(*this);
	}
}
void TypeScanner::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitFunctionStatement(const ast::Function &functionAst) {
	std::string currentModule;

	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > directivesStackTop; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(),
			    std::format(
			        "unmatched compiler directive '{}' for function '{}'",
			        directive->directiveName(), functionAst.name.getLexeme()));
		}
		directivesStack.pop_back();
	}
	std::string mangledFunctionName =
	    passes::mangling::NameMangler().mangleFunction(
	        currentModule, functionAst, linkageDirective);

	auto type = resolveType(*functionAst.returnType);
	if (functionAst.body.has_value()) {
		functionAst.body->visit(*this);
	}
}
void TypeScanner::visitIfStatement(const ast::If &ifExprAst) {
	// we do not care for the condition, only the inner body of the expression
	// and the else body if applies
	ifExprAst.thenBranch->visit(*this);
	if (ifExprAst.elseBranch.has_value()) {
		ifExprAst.elseBranch->get()->visit(*this);
	}
}
void TypeScanner::visitJumpStatement(const ast::Jump &jumpAst) {
	if (jumpAst.returnValue.has_value()) {
		jumpAst.returnValue->get()->visit(*this);
	}
}
void TypeScanner::visitVarDeclStatement(const ast::VarDecl &varDeclAst) {
	// TODO: revisit this section once we implement abstract values such as
	// modules
	// TODO: review wether we should discover variables here before type checker
}
void TypeScanner::visitMemberStatement(const ast::Member &memberAst) {
	std::string memberName = memberAst.name.lexeme;

	auto memberTypeObj =
	    resolveType(*memberAst.type).value_or(lang::Type::defineUnknownType());
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

	for (size_t i = directivesStack.size(); i > directivesStackTop; i--) {
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
	auto structObjRes = scope.findLocalStruct(structName)
	                        .value_or(util::soft_reference<lang::Struct>())
	                        .getObject();
	if (!structObjRes.has_value()) {
		messageBag.bug(structAst.getToken(),
		               std::format("could not find internal reference for {}",
		                           structName));
		return;
	}
	auto &structObj = structObjRes.value().get();
	std::vector<lang::StructMember> members;
	for (const auto &member : structAst.members) {
		member.visit(*this);
		assert(!structMemberStack.empty());
		auto memberObj = structMemberStack.back();
		structMemberStack.pop_back();

		members.push_back(memberObj);
	}

	structObj.members = members;
}
void TypeScanner::visitCompDirectiveStatement(
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
		              : directive::LinkageDirective::ManglingType::Unknonw
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
				if (directiveType.has_value()) {
					typeStack.push_back(directiveType.value());
				}
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
// Expression
void TypeScanner::visitVariableExpression(const ast::Variable &varExprAst) {
	// TODO: once we have modules support(and maybe a template system?)
	// revisit this section so we can determine if abstract variables can hold
	// values required to them
}
void TypeScanner::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitAssignExpression(const ast::Assign &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitBinaryExpression(const ast::Binary &binaryExprAst) {
	// we do not care about binary expressions as they cannot yield a new type
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
void TypeScanner::visitLiteralExpression(const ast::Literal &literalExpr) {
	// we do not care about literal expression in the early scan phase
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
void TypeScanner::visitArrayTypeExpression(const ast::ArrayType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitTupleTypeExpression(const ast::TupleType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitPointerTypeExpression(
    const ast::PointerType &pointerTypeAst) {
	auto innerType = resolveType(*pointerTypeAst.subtype)
	                     .value_or(lang::Type::defineUnknownType());
	typeStack.push_back(currentDataModel.get().definePointerType(
	    innerType, pointerTypeAst.isMutable));
}
void TypeScanner::visitNamedTypeExpression(const ast::NamedType &typeAst) {
	auto queriedType = findTypeInfo(typeAst.name.lexeme);
	if (queriedType != lang::Type::defineUnknownType()) {
		lang::Type obtainedType = queriedType;
		obtainedType.isMutable = typeAst.isMutable;
		typeStack.push_back(obtainedType);
	} else {
		messageBag.error(
		    typeAst.getToken(),
		    std::format("type not found for {}", typeAst.name.lexeme));
		typeStack.push_back(lang::Type::defineUnknownType());
	}
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

std::optional<lang::Type>
TypeScanner::findScalarTypeInfo(const std::string_view lexeme) {
	return currentDataModel.get().findScalarType(lexeme);
}
lang::Type TypeScanner::findTypeInfo(const std::string_view typeName) {
	auto scalarType = findScalarTypeInfo(typeName);
	if (scalarType) {
		return scalarType.value();
	}
	// a defined type in the source unit cannot shadow a primitive/scalar type
	auto foundStruct = currentSourceUnit.findStruct(typeName, currentScope);
	return foundStruct
	    .transform([&](auto &structObj) {
		    return currentDataModel.get().defineStructType(
		        structObj.get().structID, structObj.get().name, 0);
	    })
	    .value_or(lang::Type::defineUnknownType());
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

	for (size_t i = directivesStack.size(); i > directivesStackTop; i--) {
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

	// declare the struct first so we can bind the definition later
	if (!currentSourceUnit.declareStruct(
	        lang::Struct{
	            .opaque = true,                   // unknown implementation
	            .name = structName,               //
	            .mangledName = mangledStructName, //
	        },
	        scope)) {
		messageBag.error(structAst.getToken(), "could not declare struct");
	}
	// do not bother with declarations
	if (structAst.declaration) {
		return;
	}

	auto foundStruct = scope.findLocalStruct(structName);
	size_t structID = 0;
	if (foundStruct) {
		structID = foundStruct->getObjectId();
		assert(foundStruct->getObjectId() != 0);
		if (!foundStruct->getObject()->get().opaque) {
			messageBag.error(
			    structAst.getToken(),
			    std::format("{} is defined multiple times", structName));
		}
	}

	if (!scope.bindStruct(lang::Struct{
	        .opaque = false, // known struct
	        .structID =
	            structID, // make sure to pass the struct ID to avoid loosing it
	        .name = structName,               //
	        .mangledName = mangledStructName, //
	        .members = {}                     //
	    })) {
		messageBag.error(structAst.getToken(),
		                 std::format("could not bind struct '{}'", structName));
		return;
	}
}

} // namespace ray::compiler::passes
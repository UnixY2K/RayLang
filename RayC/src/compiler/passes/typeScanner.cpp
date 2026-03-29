
#include <cassert>
#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/trait.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/typeScanner.hpp>
#include <ray/util/copy_ptr.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::passes {

void TypeScanner::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {
	// search for structs
	for (const auto &structAst :
	     statements | std::views::transform([](const auto &statement) {
		     return dynamic_cast<const ast::Struct *>(statement.get());
	     }) | std::views::filter([](const auto *structAst) {
		     return structAst != nullptr;
	     })) {
		// TODO: refactor the compiler directives so they can be attached to
		// the related AST instead
		discoverStruct(*structAst);
	}
	// iterate each statement
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
    const ast::ExpressionStmt &expressionStmtAst) {
	expressionStmtAst.expression->visit(*this);
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
		resolveTypes(*functionAst.body);
	}
	typeStack.push_back(type);
}
void TypeScanner::visitMethodStatement(const ast::Method &methodAst) {
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
			    std::format("unmatched compiler directive '{}' for method '{}'",
			                directive->directiveName(),
			                methodAst.name.getLexeme()));
		}
		directivesStack.pop_back();
	}
	std::string mangledMethodName =
	    passes::mangling::NameMangler().mangleMethod(currentModule, methodAst,
	                                                 linkageDirective);

	std::string methodName = methodAst.name.lexeme;

	lang::Type returnType = resolveType(*methodAst.returnType.get());
	std::vector<lang::MethodParameter> parameters;
	for (const auto &parameter : methodAst.params) {
		lang::Type parameterType = resolveType(parameter);
		parameters.push_back(lang::MethodParameter{
		    .name = std::string(parameter.name.getLexeme()),
		    .parameterType = parameterType});
	}

	lang::MethodSignature methodSignature = {returnType, parameters};

	auto traitMethod = lang::Method{
	    .methodID = 0,
	    .name = methodName,
	    .mangledName = mangledMethodName,
	    .publicVisibility = methodAst.publicVisibility,
	    .signature = methodSignature,
	};

	traitMethodStack.push_back(traitMethod);
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
	if (varDeclAst.initializer.has_value()) {
		auto initializerType = resolveType(*varDeclAst.initializer->get());
	}
}
void TypeScanner::visitMemberStatement(const ast::Member &memberAst) {
	std::string memberName = memberAst.name.lexeme;

	auto memberTypeObj = resolveType(*memberAst.type);
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
void TypeScanner::visitWhileStatement(const ast::While &whileAst) {
	resolveType(*whileAst.body);
	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeScanner::visitStructStatement(const ast::Struct &structAst) {
	// process all the linkage directives to ensure they are not dangling after
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
	if (!currentSourceUnit.declareStruct(
	        lang::Struct{
	            .opaque = true,                   // unknown implementation
	            .name = std::string(structName),  //
	            .mangledName = mangledStructName, //
	        },
	        scope)) {
		messageBag.error(structAst.getToken(), "could not declare struct");
	}

	auto structObjRes = scope.findLocalStruct(structName)
	                        .value_or(util::soft_reference<lang::Struct>())
	                        .getObject();

	// TODO: change how compiler directives work
	// this is an ugly workarround to declare structs that have compiler
	// directives and were not discovered due to it

	if (structAst.declaration) {
		return;
	}

	if (!structObjRes.has_value()) {
		messageBag.bug(
		    structAst.getToken(),
		    std::format("could not find Struct internal reference for '{}'",
		                structName));
		return;
	}
	auto &structObj = structObjRes.value().get();
	std::vector<lang::StructMember> members;
	for (const auto &member : structAst.members) {
		member.visit(*this);
		if (structMemberStack.empty()) {
			messageBag.bug(
			    member.getToken(),
			    std::format("could not get struct member data for '{}'",
			                member.name.getLexeme()));
			continue;
		}
		auto memberObj = structMemberStack.back();
		structMemberStack.pop_back();

		members.push_back(memberObj);
	}

	structObj.members = members;
}
void TypeScanner::visitTraitStatement(const ast::Trait &traitAst) {
	// process all the linkage directives to ensure they are not dangling after
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

	auto traitName = traitAst.name.getLexeme();
	std::string currentModule;
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleTrait(currentModule, traitAst);

	auto &scope = currentScope.get();
	if (!currentSourceUnit.declareTrait(
	        lang::Trait{
	            .name = std::string(traitName),   //
	            .mangledName = mangledStructName, //
	        },
	        scope)) {
		messageBag.error(traitAst.getToken(), "could not declare trait");
	}

	auto traitObjRes = scope.findLocalTrait(traitName)
	                       .value_or(util::soft_reference<lang::Trait>())
	                       .getObject();

	if (!traitObjRes.has_value()) {
		messageBag.bug(
		    traitAst.getToken(),
		    std::format("could not find Trait internal reference for '{}'",
		                traitName));
		return;
	}
	auto &traitObj = traitObjRes.value().get();
	std::vector<lang::Method> methods;
	for (const auto &method : traitAst.methods) {
		method.visit(*this);
		if (traitMethodStack.empty()) {
			messageBag.bug(
			    method.getToken(),
			    std::format("could not get trait method data for '{}'",
			                method.name.getLexeme()));
			continue;
		}
		auto traitObj = traitMethodStack.back();
		traitMethodStack.pop_back();

		methods.push_back(traitObj);
	}

	traitObj.methods = methods;
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
// Expression
void TypeScanner::visitVariableExpression(const ast::Variable &varExprAst) {
	// TODO: once we have modules support(and maybe a template system?)
	// revisit this section so we can determine if abstract variables can hold
	// values required to them
	auto foundVariable =
	    currentScope.get()
	        .findVariable(varExprAst.name.lexeme)
	        .transform([](const util::soft_reference<lang::Symbol> &symbolRef)
	                       -> lang::Symbol {
		        lang::Symbol returnSymbol =
		            symbolRef.getObject()
		                .transform(
		                    [](const std::reference_wrapper<const lang::Symbol>
		                           &symbolRef) -> lang::Symbol {
			                    return symbolRef.get();
		                    })
		                .value_or(lang::Symbol::defineUnknownSymbol());
		        return returnSymbol;
	        })
	        .value_or(lang::Symbol::defineUnknownSymbol());
	typeStack.push_back(foundVariable.innerType);
}
void TypeScanner::visitIntrinsicExpression(const ast::Intrinsic &intrinsicAst) {
	switch (intrinsicAst.intrinsic) {
	case ast::IntrinsicType::INTR_SIZEOF: {
		typeStack.push_back(currentDataModel.get().getScalarType(
		    environment::DataModel::ScalarTypeKind::ssizeScalar));
		break;
	}
	case ast::IntrinsicType::INTR_IMPORT: {
		// TODO: return modulequery type so the module can be scanned
		messageBag.error(
		    intrinsicAst.getToken(),
		    std::format("{} not implemented", __PRETTY_FUNCTION__));
		break;
	}
	case ast::IntrinsicType::INTR_UNKNOWN:
		break;
	}
}
void TypeScanner::visitAssignExpression(const ast::Assign &assignAst) {
	assignAst.rhs->visit(*this);
}
void TypeScanner::visitBinaryExpression(const ast::Binary &binaryExprAst) {
	// we do not care about binary expressions as they cannot yield a new type
}
void TypeScanner::visitCallExpression(const ast::Call &callAst) {
	// the type checker is responsible for verifying the types
	for (const auto &argument : callAst.arguments) {
		argument->visit(*this);
	}
}
void TypeScanner::visitIntrinsicCallExpression(
    const ast::IntrinsicCall &intrinsicCallAst) {
	// TODO: review this section later for a module system
	auto type = resolveType(*intrinsicCallAst.callee);

	for (auto &argument : intrinsicCallAst.arguments) {
		argument->visit(*this);
	}
}
void TypeScanner::visitGetExpression(const ast::Get &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitGroupingExpression(const ast::Grouping &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitLiteralExpression(const ast::Literal &literalAst) {
	switch (literalAst.kind.type) {

	case Token::TokenType::TOKEN_STRING: {
		const auto baseType =
		    currentDataModel.get().findScalarType("u8").value();
		// literal strings are not mutable
		const auto arrayType =
		    currentDataModel.get().definePointerType(baseType, false);
		typeStack.push_back(arrayType);
		break;
	}
	case Token::TokenType::TOKEN_NUMBER: {
		auto type = currentDataModel.get().getNumberLiteralType(
		    literalAst.token.lexeme);
		if (!type.has_value()) {
			messageBag.error(
			    literalAst.getToken(),
			    std::format("'{}' cannot be hold in any scalar number type",
			                literalAst.getToken().getLexeme()));
			return;
		}
		typeStack.push_back(type.value());
		break;
	}
	case Token::TokenType::TOKEN_CHAR: {
		// any char token is a u8 character, not a unicode encode character
		// so only ASCII characters allowed
		const std::string_view character = literalAst.value;
		if (character.size() > 1) {
			messageBag.error(
			    literalAst.getToken(),
			    std::format("'{}' is not a valid char literal type",
			                literalAst.getToken().getLexeme()));
			break;
		}
		typeStack.push_back(
		    currentDataModel.get().findScalarType("u8").value());
		break;
	}
	default:
		messageBag.error(literalAst.getToken(),
		                 std::format("'{}' is not a valid literal type",
		                             literalAst.getToken().getLexeme()));
		break;
	}
}
void TypeScanner::visitLogicalExpression(const ast::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitSetExpression(const ast::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitUnaryExpression(const ast::Unary &unaryAst) {
	// TODO: rework this section once operator overload is implemented
	// for now we assume the same type is returned
	auto innerType = resolveType(*unaryAst.expr);
	return typeStack.push_back(innerType);
}
void TypeScanner::visitArrayAccessExpression(
    const ast::ArrayAccess &arrayAccessAst) {
	// TODO: remove this hack in the future once we convert to an cleaner AST
	auto arrayType = resolveType(*arrayAccessAst.array);
	auto indexType = resolveType(*arrayAccessAst.index);
	lang::Type innerType =
	    arrayType.subtype
	        .transform([](util::copy_ptr<lang::Type> &typePtr) -> lang::Type {
		        assert(typePtr.get() != nullptr);
		        return *typePtr.get();
	        })
	        .value_or(lang::Type::defineUnknownType());
	typeStack.push_back(innerType);
}
void TypeScanner::visitArrayTypeExpression(const ast::ArrayType &arrayTypeAst) {
	auto innerType = resolveType(*arrayTypeAst.subType);

	typeStack.push_back(currentDataModel.get().definePointerType(
	    innerType, arrayTypeAst.isMutable));
}
void TypeScanner::visitTupleTypeExpression(const ast::TupleType &tupleAst) {
	if (tupleAst.expressions.empty()) {
		typeStack.push_back(currentDataModel.get().getUnitType());
		return;
	}
	messageBag.error(
	    tupleAst.getToken(),
	    std::format("{} not implemented for tuples", __PRETTY_FUNCTION__));
}
void TypeScanner::visitPointerTypeExpression(
    const ast::PointerType &pointerTypeAst) {
	auto innerType = resolveType(*pointerTypeAst.subtype);
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
void TypeScanner::visitCastExpression(const ast::Cast &castAst) {
	castAst.expression->visit(*this);
	typeStack.push_back(resolveType(*castAst.type));
}
void TypeScanner::visitParameterExpression(const ast::Parameter &parameterAst) {
	lang::Type parameterType = resolveType(*parameterAst.type.get());
	typeStack.push_back(parameterType);
}

lang::Type TypeScanner::resolveType(const ast::Statement &statement) {
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

	return types.size() > 0 ? types[0] : lang::Type::defineUnknownType();
}
lang::Type TypeScanner::resolveType(const ast::Expression &expression) {
	auto types = resolveTypes(expression);

	if (types.size() > 1) {
		messageBag.bug(expression.getToken(),
		               std::format("'{}' yield multiple values",
		                           expression.variantName()));
	}

	return types.size() > 0 ? types[0] : lang::Type::defineUnknownType();
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
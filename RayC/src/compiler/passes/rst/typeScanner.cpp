
#include <cassert>
#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

#include <ray/compiler/passes/typeScanner.hpp>

#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/trait.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/syntax/rst/Expression.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>
#include <ray/util/copy_ptr.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::passes {

void TypeScanner::resolve(const syntax::rst::Block &block) {
	// search for structs
	const auto &statements = block.statements;
	for (const auto &structRst :
	     statements | std::views::transform([](const auto &statement) {
		     return dynamic_cast<const syntax::rst::Struct *>(statement.get());
	     }) | std::views::filter([](const auto *structRst) {
		     return structRst != nullptr;
	     })) {
		discoverStruct(*structRst);
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

void TypeScanner::visitBlockStatement(const syntax::rst::Block &blockRst) {
	auto &parentScope = currentScope.get();
	currentScope = parentScope.makeChildScope();
	std::vector<lang::Type> returnTypes;

	for (const auto &astStatement : blockRst.statements) {
		auto statementTypes = resolveTypes(*astStatement.get());
		while (!statementTypes.empty()) {
			auto returnType = statementTypes.back();
			statementTypes.pop_back();
			returnTypes.push_back(returnType);
		}
	}

	for (const auto &returnType : returnTypes) {
		if (returnType != lang::Type::defineStmtType()) {
			typeStack.push_back(returnType);
		}
	}

	currentScope = parentScope;
}
void TypeScanner::visitTerminalExpressionStatement(
    const syntax::rst::TerminalExpression &terminalExprRst) {
	if (terminalExprRst.expression.has_value()) {
		terminalExprRst.expression->get()->visit(*this);
	}
}
void TypeScanner::visitExpressionStatementStatement(
    const syntax::rst::ExpressionStatement &expressionStmtRst) {
	expressionStmtRst.expression->visit(*this);
}
void TypeScanner::visitFunctionStatement(
    const syntax::rst::Function &functionRst) {
	std::string currentModule = "root";

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
			        directive->directiveName(), functionRst.name.getLexeme()));
		}
		directivesStack.pop_back();
	}
	std::string mangledFunctionName =
	    passes::mangling::NameMangler().mangleFunction(
	        currentModule, functionRst, linkageDirective);

	auto returnType = resolveType(*functionRst.returnType->get());
	// update the function signature
	auto &functionsTable = currentSourceUnit.getFunctions();
	if (!functionsTable.contains(functionRst.functionId)) {
		messageBag.bug(
		    functionRst.getToken(),
		    std::format("could not find internal function via ID#{} for '{}'",
		                functionRst.functionId, mangledFunctionName));
	} else {
		auto &functionDeclaration = functionsTable.at(functionRst.functionId);
		functionDeclaration.signature.returnType = returnType;
		// also resolve the function parameter list
		std::vector<lang::FunctionParameter> resolvedParameters;
		for (size_t index = 0; index < functionRst.params.size(); index++) {
			const auto &parameter = functionRst.params.at(index);
			auto paramType = resolveType(parameter);
			functionDeclaration.signature.parameters.at(index).parameterType = paramType;
		}
	}

	if (functionRst.body.has_value()) {
		discardTypes(*functionRst.body->get());
	}
	typeStack.push_back(returnType);
}
void TypeScanner::visitIfStatement(const syntax::rst::If &ifExprRst) {
	// we do not care for the condition, only the inner body of the expression
	// and the else body if applies
	ifExprRst.thenBranch->visit(*this);
	if (ifExprRst.elseBranch.has_value()) {
		ifExprRst.elseBranch->get()->visit(*this);
	}
}
void TypeScanner::visitJumpStatement(const syntax::rst::Jump &jumpRst) {
	if (jumpRst.returnValue.has_value()) {
		jumpRst.returnValue->get()->visit(*this);
	}
}
void TypeScanner::visitVarDeclStatement(
    const syntax::rst::VarDecl &varDeclRst) {
	if (varDeclRst.initializer.has_value()) {
		auto initializerType = resolveType(*varDeclRst.initializer->get());
	}
}
void TypeScanner::visitMemberStatement(const syntax::rst::Member &memberRst) {
	std::string memberName = memberRst.name.lexeme;

	auto memberTypeObj = resolveType(*memberRst.type);
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
void TypeScanner::visitWhileStatement(const syntax::rst::While &whileRst) {
	discardTypes(*whileRst.body);
	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeScanner::visitStructStatement(const syntax::rst::Struct &structRst) {
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

	auto structName = structRst.name.getLexeme();
	std::string currentModule = "root";
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structRst,
	                                                 linkageDirective);
	auto &scope = currentScope.get();
	if (!currentSourceUnit.declareStruct(
	        lang::Struct{
	            .opaque = true,                   // unknown implementation
	            .name = std::string(structName),  //
	            .mangledName = mangledStructName, //
	        },
	        scope)) {
		messageBag.error(structRst.getToken(), "could not declare struct");
	}

	auto structObjRes = scope.findLocalStruct(structName)
	                        .value_or(util::soft_reference<lang::Struct>())
	                        .getObject();

	// TODO: change how compiler directives work
	// this is an ugly workarround to declare structs that have compiler
	// directives and were not discovered due to it

	if (structRst.declaration) {
		return;
	}

	if (!structObjRes.has_value()) {
		messageBag.bug(
		    structRst.getToken(),
		    std::format("could not find Struct internal reference for '{}'",
		                structName));
		return;
	}
	auto &structObj = structObjRes.value().get();
	std::vector<lang::StructMember> members;
	for (const auto &member : structRst.members) {
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
void TypeScanner::visitPlaceholderStatement(
    const syntax::rst::Placeholder &placeholderRST) {}

// Expression
void TypeScanner::visitVariableExpression(
    const syntax::rst::Variable &varExprRst) {
	// TODO: once we have modules support(and maybe a template system?)
	// revisit this section so we can determine if abstract variables can hold
	// values required to them
	auto foundVariable =
	    currentScope.get()
	        .findVariable(varExprRst.name.lexeme)
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
void TypeScanner::visitIntrinsicExpression(
    const syntax::rst::Intrinsic &intrinsicRst) {
	switch (intrinsicRst.intrinsic) {
	case syntax::common::IntrinsicType::INTR_SIZEOF: {
		typeStack.push_back(currentDataModel.get().getScalarType(
		    environment::DataModel::ScalarTypeKind::ssizeScalar));
		break;
	}
	case syntax::common::IntrinsicType::INTR_IMPORT: {
		// TODO: return modulequery type so the module can be scanned
		messageBag.error(
		    intrinsicRst.getToken(),
		    std::format("{} not implemented", __PRETTY_FUNCTION__));
		break;
	}
	case syntax::common::IntrinsicType::INTR_UNKNOWN:
		break;
	}
}
void TypeScanner::visitAssignExpression(const syntax::rst::Assign &assignRst) {
	assignRst.rhs->visit(*this);
}
void TypeScanner::visitBinaryExpression(
    const syntax::rst::Binary &binaryExprRst) {
	// TODO: once operator overload is implemented
	// make use of the scanning to resolve its type
	auto leftType = resolveType(*binaryExprRst.left);
	auto rightType = resolveType(*binaryExprRst.right);

	auto op = binaryExprRst.op;
	// TODO: once we start supporting operator overload this should be done by
	// lookup of the overloads and get the return type of it
	switch (op.type) {
	case Token::TokenType::TOKEN_PLUS:
	case Token::TokenType::TOKEN_MINUS:
	case Token::TokenType::TOKEN_STAR:
	case Token::TokenType::TOKEN_SLASH:
	case Token::TokenType::TOKEN_PERCENT:
	case Token::TokenType::TOKEN_AMPERSAND:
	case Token::TokenType::TOKEN_PIPE:
	case Token::TokenType::TOKEN_CARET:
	case Token::TokenType::TOKEN_LESS_LESS:
		// currently assume the the type is the same as left expression type
		typeStack.push_back(leftType);
		break;
	case Token::TokenType::TOKEN_GREAT_GREAT:
	case Token::TokenType::TOKEN_EQUAL_EQUAL:
	case Token::TokenType::TOKEN_BANG_EQUAL:
	case Token::TokenType::TOKEN_LESS:
	case Token::TokenType::TOKEN_GREAT:
	case Token::TokenType::TOKEN_LESS_EQUAL:
	case Token::TokenType::TOKEN_GREAT_EQUAL:
		typeStack.push_back(findScalarTypeInfo("bool").value());
		break;
	default:
		messageBag.error(binaryExprRst.op,
		                 std::format("'{}' is not a supported binary operation",
		                             op.getLexeme()));
	}
}
void TypeScanner::visitCallExpression(const syntax::rst::Call &callRst) {
	// the type checker is responsible for verifying the types
	// for (const auto &argument : callRst.arguments) {
	//	argument->visit(*this);
	//}
	auto returnType = resolveType(*callRst.callee.get());
	typeStack.push_back(returnType);
}
void TypeScanner::visitIntrinsicCallExpression(
    const syntax::rst::IntrinsicCall &intrinsicCallRst) {
	// TODO: review this section later for a module system
	auto type = resolveType(*intrinsicCallRst.callee);

	for (auto &argument : intrinsicCallRst.arguments) {
		argument->visit(*this);
	}
}
void TypeScanner::visitGetExpression(const syntax::rst::Get &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitGroupingExpression(
    const syntax::rst::Grouping &groupingRst) {
	auto returnType = resolveType(*groupingRst.expression.get());
	typeStack.push_back(returnType);
}
void TypeScanner::visitLiteralExpression(
    const syntax::rst::Literal &literalRst) {
	switch (literalRst.kind.type) {

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
		    literalRst.token.lexeme);
		if (!type.has_value()) {
			messageBag.error(
			    literalRst.getToken(),
			    std::format("'{}' cannot be hold in any scalar number type",
			                literalRst.getToken().getLexeme()));
			return;
		}
		typeStack.push_back(type.value());
		break;
	}
	case Token::TokenType::TOKEN_CHAR: {
		// any char token is a u8 character, not a unicode encode character
		// so only ASCII characters allowed
		const std::string_view character = literalRst.value;
		if (character.size() > 1) {
			messageBag.error(
			    literalRst.getToken(),
			    std::format("'{}' is not a valid char literal type",
			                literalRst.getToken().getLexeme()));
			break;
		}
		typeStack.push_back(
		    currentDataModel.get().findScalarType("u8").value());
		break;
	}
	default:
		messageBag.error(literalRst.getToken(),
		                 std::format("'{}' is not a valid literal type",
		                             literalRst.getToken().getLexeme()));
		break;
	}
}
void TypeScanner::visitLogicalExpression(const syntax::rst::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitSetExpression(const syntax::rst::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeScanner::visitUnaryExpression(const syntax::rst::Unary &unaryRst) {
	// TODO: rework this section once operator overload is implemented
	// for now we assume the same type is returned
	auto innerType = resolveType(*unaryRst.expr);
	return typeStack.push_back(innerType);
}
void TypeScanner::visitArrayAccessExpression(
    const syntax::rst::ArrayAccess &arrayAccessRst) {
	// TODO: remove this hack in the future once we convert to an cleaner AST
	auto arrayType = resolveType(*arrayAccessRst.array);
	auto indexType = resolveType(*arrayAccessRst.index);
	lang::Type innerType =
	    arrayType.subtype
	        .transform([](util::copy_ptr<lang::Type> &typePtr) -> lang::Type {
		        assert(typePtr.get() != nullptr);
		        return *typePtr.get();
	        })
	        .value_or(lang::Type::defineUnknownType());
	typeStack.push_back(innerType);
}
void TypeScanner::visitArrayTypeExpression(
    const syntax::rst::ArrayType &arrayTypeRst) {
	auto innerType = resolveType(*arrayTypeRst.subType);

	typeStack.push_back(currentDataModel.get().definePointerType(
	    innerType, arrayTypeRst.isMutable));
}
void TypeScanner::visitTupleTypeExpression(
    const syntax::rst::TupleType &tupleRst) {
	if (tupleRst.expressions.empty()) {
		typeStack.push_back(currentDataModel.get().getUnitType());
		return;
	}
	messageBag.error(
	    tupleRst.getToken(),
	    std::format("{} not implemented for tuples", __PRETTY_FUNCTION__));
}
void TypeScanner::visitPointerTypeExpression(
    const syntax::rst::PointerType &pointerTypeRst) {
	auto innerType = resolveType(*pointerTypeRst.subtype);
	typeStack.push_back(currentDataModel.get().definePointerType(
	    innerType, pointerTypeRst.isMutable));
}
void TypeScanner::visitNamedTypeExpression(
    const syntax::rst::NamedType &typeRst) {
	auto queriedType = findTypeInfo(typeRst.name.lexeme);
	if (queriedType != lang::Type::defineUnknownType()) {
		lang::Type obtainedType = queriedType;
		obtainedType.isMutable = typeRst.isMutable;
		typeStack.push_back(obtainedType);
	} else {
		messageBag.error(
		    typeRst.getToken(),
		    std::format("type not found for {}", typeRst.name.lexeme));
		typeStack.push_back(lang::Type::defineUnknownType());
	}
}
void TypeScanner::visitCastExpression(const syntax::rst::Cast &castRst) {
	discardTypes(*castRst.expression.get());
	typeStack.push_back(resolveType(*castRst.type));
}
void TypeScanner::visitParameterExpression(
    const syntax::rst::Parameter &parameterRst) {
	lang::Type parameterType = resolveType(*parameterRst.type.get());
	typeStack.push_back(parameterType);
}
void TypeScanner::visitPlaceHolderExpression(
    const syntax::rst::PlaceHolder &placeholderRST) {}

lang::Type TypeScanner::resolveType(const syntax::rst::Statement &statement) {
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

	return types.size() > 0 ? types[0] : lang::Type::defineUnknownType();
}
lang::Type TypeScanner::resolveType(const syntax::rst::Expression &expression) {
	auto types = resolveTypes(expression);

	if (types.size() > 1) {
		messageBag.bug(expression.getToken(),
		               std::format("'{}' yield multiple values",
		                           expression.variantName()));
	}

	return types.size() > 0 ? types[0] : lang::Type::defineUnknownType();
}
std::vector<lang::Type>
TypeScanner::resolveTypes(const syntax::rst::Statement &statement) {
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
TypeScanner::resolveTypes(const syntax::rst::Expression &expression) {
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

void TypeScanner::discardTypes(const syntax::rst::Statement &statement) {
	size_t tsSize = typeStack.size();
	statement.visit(*this);
	while (typeStack.size() > tsSize) {
		typeStack.pop_back();
	}
}
void TypeScanner::discardTypes(const syntax::rst::Expression &expression) {
	size_t tsSize = typeStack.size();
	expression.visit(*this);
	while (typeStack.size() > tsSize) {
		typeStack.pop_back();
	}
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

void TypeScanner::discoverStruct(const syntax::rst::Struct &structRst) {
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

	std::string structName = std::string(structRst.name.getLexeme());
	std::string currentModule = "root";
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structRst,
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
		messageBag.error(structRst.getToken(), "could not declare struct");
	}
	// do not bother with declarations
	if (structRst.declaration) {
		return;
	}

	auto foundStruct = scope.findLocalStruct(structName);
	size_t structID = 0;
	if (foundStruct) {
		structID = foundStruct->getObjectId();
		assert(foundStruct->getObjectId() != 0);
		if (!foundStruct->getObject()->get().opaque) {
			messageBag.error(
			    structRst.getToken(),
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
		messageBag.error(structRst.getToken(),
		                 std::format("could not bind struct '{}'", structName));
		return;
	}
}

} // namespace ray::compiler::passes
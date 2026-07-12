#include <cassert>
#include <format>

#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/passes/rst/typeChecker.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>

namespace ray::compiler::passes::rst {

void TypeChecker::resolve(syntax::rst::Block &rootBlock) {
	rootBlock.visit(*this);
}

bool TypeChecker::hasFailed() const { return messageBag.failed(); }
const MessageBag &TypeChecker::getMessageBag() const { return messageBag; }

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
    const syntax::rst::TerminalExpression &terminalExpressionRST) {
	if (terminalExpressionRST.expression.has_value()) {
		const auto &returnExpr = *terminalExpressionRST.expression.value();
		auto returnType = resolveType(returnExpr);
		if (returnType.has_value()) {
			typeStack.push_back(returnType.value());
		} else {
			messageBag.error(
			    returnExpr.getToken(),
			    std::format("{} child expression did not yield a value '{}'",
			                terminalExpressionRST.variantName(),
			                returnExpr.variantName()));
		}
		return;
	}

	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitExpressionStatementStatement(
    const syntax::rst::ExpressionStatement &expressionStatementRST) {
	resolveType(*expressionStatementRST.expression);
	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitFunctionStatement(
    const syntax::rst::Function &functionRST) {

	auto declarationResult = resolveFunctionDeclaration(functionRST);
	if (!declarationResult.has_value()) {
		messageBag.error(
		    functionRST.getToken(),
		    std::format("could not resolve function declaration for '{}'",
		                functionRST.name.getLexeme()));
	} else {
		const auto &functionDeclaration = declarationResult.value();

		// declaration was already defined, so it does not require to be defined
		// again, just the body
		if (functionRST.body.has_value()) {

			lang::Scope &parentScope = currentScope;
			currentScope = currentScope.get().makeChildScope();
			// add functions to the current scope and validate that each
			for (const auto &param : functionDeclaration.signature.parameters) {
				// TODO: variable definitions should be done at an earlier stage
				lang::Symbol paramSymbol{
				    .name = param.name,
				    .mangledName = param.name,
				    .innerType = param.parameterType,
				    .type = lang::Symbol::SymbolType::Parameter,
				    .internal = false,
				};
				if (!currentSourceUnit.declareLocalVariable(
				        paramSymbol, getCurrentScope())) {
					messageBag.bug(
					    functionRST.token,
					    std::format("parameter '{} 'could not be defined",
					                paramSymbol.name));
				}
			}

			auto type = resolveType(*functionRST.body->get())
			                .value_or(currentDataModel.get().getUnitType());
			if (type == lang::Type::defineStmtType()) {
				type = lang::Type::defineUnitType();
			}

			if (!type.coercercesInto(
			        functionDeclaration.signature.returnType) &&
			    // main is the only function allowed to not return anything
			    functionDeclaration.mangledName != "main") {
				messageBag.error(
				    functionRST.body->get()->getToken(),
				    std::format(
				        "unmatched body return type with (body)'{}' vs (declaration)'{}'",
				        type.name,
				        functionDeclaration.signature.returnType.name));
			}

			currentScope = parentScope;
		}

		std::vector<util::copy_ptr<lang::Type>> paramTypes;
		for (const auto &param : functionDeclaration.signature.parameters) {
			paramTypes.push_back(
			    util::copy_ptr<lang::Type>(param.parameterType));
		}

		auto functionType = currentDataModel.get().defineFunctionType(
		    functionDeclaration.signature.returnType, paramTypes);
		typeStack.push_back(functionType);
	}
}
void TypeChecker::visitIfStatement(const syntax::rst::If &ifStmtRST) {
	auto conditionType = resolveType(*ifStmtRST.condition);
	if (!conditionType.has_value()) {
		messageBag.error(ifStmtRST.condition->getToken(),
		                 "non boolean condition");
	} else {
		auto boolType = findScalarTypeInfo("bool");
		// for now lets just stricly validate if is the same
		// TODO: enable coercions
		if (!(conditionType->coercercesInto(boolType.value()))) {
			messageBag.error(ifStmtRST.condition->getToken(),
			                 "condition does not coerce into a bool type");
		}
	}
	auto thenRType = resolveType(*ifStmtRST.thenBranch);
	auto thenType = thenRType.has_value() ? thenRType.value()
	                                      : lang::Type::defineStmtType();
	if (ifStmtRST.elseBranch.has_value()) {
		auto elseRType = resolveType(*ifStmtRST.elseBranch.value());
		auto elseType = elseRType.has_value() ? elseRType.value()
		                                      : lang::Type::defineStmtType();
		// the types should match
		if (!(thenType == elseType)) {
			messageBag.error(
			    ifStmtRST.getToken(),
			    std::format("code branches have different types ({}|{})",
			                thenType.name, elseType.name));
		}
	}

	// return whatever our internal evaluation yield

	typeStack.push_back(thenType);
}
void TypeChecker::visitJumpStatement(
    const syntax::rst::Jump &jumpStatementRST) {
	// if our expression is a return we need to return its optional value
	// for anything else we do not care about its type
	if (jumpStatementRST.token.type == Token::TokenType::TOKEN_RETURN) {
		if (jumpStatementRST.returnValue.has_value()) {
			auto type = resolveType(*jumpStatementRST.returnValue.value());
			if (type.has_value()) {
				typeStack.push_back(type.value());
				return;
			}
		}
	}
	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitVarDeclStatement(
    const syntax::rst::VarDecl &variableDeclRST) {
	auto variableType = lang::Type{};

	if (variableDeclRST.type->getToken().type !=
	    Token::TokenType::TOKEN_UNINITIALIZED) {
		const auto &explicitType = variableDeclRST.type;
		std::string_view typeName = explicitType->getToken().lexeme;
		auto foundType = resolveType(*explicitType);
		if (!foundType.has_value()) {
			messageBag.error(
			    variableDeclRST.type->getToken(),
			    std::format("'{}' does not name an existing type", typeName));
		} else {
			variableType = foundType.value();
		}
	}

	if (variableDeclRST.initializer.has_value()) {
		auto &initializer = *variableDeclRST.initializer.value().get();
		auto initType = resolveType(initializer);
		if (!initType.has_value()) {
			messageBag.error(
			    initializer.getToken(),
			    std::format(
			        "inialization expression did not yield a type for '{}'",
			        initializer.getToken().getLexeme()));
		} else {
			const auto initializationType = initType.value();
			if (!variableType.isInitialized()) {
				variableType = initializationType;
			} else if (!initializationType.coercercesInto(variableType)) {
				messageBag.error(
				    variableDeclRST.getToken(),
				    std::format(
				        "variable initialization type does not match with explicit type for '{}': '{}' vs '{}'",
				        variableDeclRST.getToken().getLexeme(),
				        variableType.name, initializationType.name));
			}
		}
	}

	if (variableType.isInitialized()) {
		lang::Symbol variableSymbol{
		    .name = variableDeclRST.name.lexeme,
		    .mangledName = variableDeclRST.name.lexeme,
		    .innerType = variableType,
		    .type = lang::Symbol::SymbolType::Parameter,
		    .internal = false,
		};

		if (!currentSourceUnit.declareLocalVariable(variableSymbol,
		                                            getCurrentScope())) {
			messageBag.bug(variableDeclRST.getToken(),
			               std::format("variable '{} 'could not be defined",
			                           variableSymbol.name));
		}
		// typeStack.push_back(variableType);
		return;
	}

	messageBag.error(
	    variableDeclRST.getToken(),
	    "variable does not have a valid type assigned nor an valid initialization");
}
void TypeChecker::visitMemberStatement(const syntax::rst::Member &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitWhileStatement(
    const syntax::rst::While &whileStatementRST) {
	auto conditionType = resolveType(*whileStatementRST.condition);
	if (!conditionType.has_value()) {
		messageBag.error(whileStatementRST.condition->getToken(),
		                 "non boolean condition");
	} else {
		auto boolType = findScalarTypeInfo("bool");
		// for now lets just stricly validate if is the same
		// TODO: enable coercions
		if (!(conditionType->coercercesInto(boolType.value()))) {
			messageBag.error(whileStatementRST.condition->getToken(),
			                 "condition does not coerce into a bool type");
		}
	}

	const auto type = resolveType(*whileStatementRST.body)
	                      .value_or(lang::Type::defineStmtType());

	typeStack.push_back(type);
}
void TypeChecker::visitStructStatement(const syntax::rst::Struct &structRST) {
	std::optional<directive::LinkageDirective> linkageDirective;

	std::string structName = std::string(structRST.name.getLexeme());
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(
	        currentSourceUnit.packageName, structRST, linkageDirective);
	// TODO: verify members of the struct
}
void TypeChecker::visitPlaceholderStatement(
    const syntax::rst::Placeholder &value) {}

// expression visitor
void TypeChecker::visitVariableExpression(
    const syntax::rst::Variable &variableExprRst) {
	auto foundVariable =
	    getCurrentScope().findVariable(variableExprRst.name.lexeme);
	if (foundVariable.has_value()) {
		typeStack.push_back(foundVariable.value().getObject()->get().innerType);
		return;
	}

	// we just keep a cound of at most 2 to see if we return its type pointer or
	// an overloadedFunction Type
	lang::Type functionType;
	for (const auto &functionDeclarationRef :
	     currentSourceUnit.findFunctionDeclarations(variableExprRst.name.lexeme,
	                                                getCurrentScope())) {
		assert(functionDeclarationRef.getObject().has_value());
		const auto &functionDeclaration =
		    functionDeclarationRef.getObject()->get();
		if (!functionType.isInitialized()) {
			// TODO: resolve function signature before getting its type
			// as it may have not been evaluated yet
			functionType =
			    functionDeclaration.signature.getFunctionType(currentDataModel);
		} else {
			functionType =
			    functionDeclaration.signature.getOverloadedFunctionType(
			        currentDataModel);
			break;
		}
	}

	if (functionType.isInitialized()) {
		typeStack.push_back(functionType);
		return;
	}

	// check if is a known type
	auto foundType = findTypeInfo(variableExprRst.name.getLexeme());
	if (foundType.has_value()) {
		typeStack.push_back(foundType.value());
		return;
	}

	messageBag.error(variableExprRst.getToken(),
	                 std::format("unknown symbol '{}'",
	                             variableExprRst.getToken().getLexeme()));

	typeStack.push_back(lang::Type::defineUnknownType());
	return;
}
void TypeChecker::visitIntrinsicExpression(
    const syntax::rst::Intrinsic &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitAssignExpression(
    const syntax::rst::Assign &assignExpresssionRST) {
	auto leftType = resolveType(*assignExpresssionRST.lhs);
	auto rightType = resolveType(*assignExpresssionRST.rhs);

	if (!(leftType.has_value() && rightType.has_value())) {
		if (!leftType.has_value()) {
			messageBag.error(
			    assignExpresssionRST.lhs->getToken(),
			    std::format("left expression did not yield a value"));
		}

		if (!rightType.has_value()) {
			messageBag.error(
			    assignExpresssionRST.rhs->getToken(),
			    std::format("right expression did not yield a value"));
			return;
		}
		return;
	}

	auto op = assignExpresssionRST.assignmentOp;
	// TODO: once we start supporting operator overload this should be done by
	// lookup of the overloads and get the return type of it
	switch (op.type) {
	case Token::TokenType::TOKEN_EQUAL:
	case Token::TokenType::TOKEN_PLUS_EQUAL:
	case Token::TokenType::TOKEN_MINUS_EQUAL:
	case Token::TokenType::TOKEN_STAR_EQUAL:
	case Token::TokenType::TOKEN_SLASH_EQUAL:
	case Token::TokenType::TOKEN_PERCENT_EQUAL:
	case Token::TokenType::TOKEN_AMPERSAND_EQUAL:
	case Token::TokenType::TOKEN_PIPE_EQUAL:
	case Token::TokenType::TOKEN_CARET_EQUAL:
	case Token::TokenType::TOKEN_LESS_LESS_EQUAL:
	case Token::TokenType::TOKEN_GREAT_GREAT_EQUAL:
		// for now just return the same type as lhs
		typeStack.push_back(leftType.value());
		break;
	default:
		messageBag.error(
		    op, std::format("'{}' is not a supported assignment operation",
		                    op.getLexeme()));
		break;
	}
}
void TypeChecker::visitBinaryExpression(
    const syntax::rst::Binary &binaryExprRst) {
	auto leftType = resolveType(*binaryExprRst.left);
	auto rightType = resolveType(*binaryExprRst.right);

	if (!(leftType.has_value() && rightType.has_value())) {
		if (!leftType.has_value()) {
			messageBag.error(
			    binaryExprRst.left->getToken(),
			    std::format("left expression did not yield a value"));
		}

		if (!rightType.has_value()) {
			messageBag.error(
			    binaryExprRst.right->getToken(),
			    std::format("right expression did not yield a value"));
		}
		return;
	}

	auto op = binaryExprRst.op;
	// TODO: once we start supporting operator overload this should be done
	// by lookup of the overloads and get the return type of it
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
		// currently assume the the type is the same as left expression
		// type
		typeStack.push_back(leftType.value());
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
void TypeChecker::visitCallExpression(const syntax::rst::Call &callExprRst) {
	auto calleeTypeResult = resolveType(*callExprRst.callee);
	if (!calleeTypeResult.has_value()) {
		messageBag.error(
		    callExprRst.getToken(),
		    std::format("unknown callee type for {}",
		                callExprRst.callee->getToken().getLexeme()));
		return;
	}
	auto calleeType = calleeTypeResult.value();

	switch (calleeType.getKind()) {

	case lang::TypeKind::pointer: {
		if (!calleeType.signature.has_value()) {
			messageBag.error(
			    callExprRst.getToken(),
			    std::format(
			        "expression does not have a valid signature for '{}'",
			        callExprRst.getToken().getLexeme()));
			break;
		}
		// valid signature
		if (callExprRst.arguments.size() != calleeType.signature->size()) {
			messageBag.error(
			    callExprRst.getToken(),
			    std::format(
			        "parameter number mismatch for '{}', provided {}, but {} were required",
			        callExprRst.getToken().getLexeme(),
			        callExprRst.arguments.size(),
			        calleeType.signature->size()));
		}
		for (size_t i = 0; i < callExprRst.arguments.size(); i++) {
			const auto &callerParamExpr = *callExprRst.arguments[i];
			const auto callerParamTypeResult = resolveType(callerParamExpr);
			const auto &calleeParamType = *calleeType.signature.value()[i];

			if (!callerParamTypeResult.has_value()) {
				messageBag.error(
				    callerParamExpr.getToken(),
				    std::format("argument does not yield a valid type for '{}'",
				                callerParamExpr.getToken().getLexeme()));
				return;
			}
			const auto &callerParamType = callerParamTypeResult.value();

			if (!callerParamType.coercercesInto(calleeParamType)) {
				messageBag.error(
				    callerParamExpr.getToken(),
				    std::format(
				        "argument #{} '{}' does not coerce the expected type (caller){} vs (callee){}",
				        i, callExprRst.getToken().getLexeme(),
				        callerParamType.name, calleeParamType.name));
				continue;
			}
		}

		break;
	}
	case lang::TypeKind::scalar:
	case lang::TypeKind::aggregate: {
		messageBag.error(callExprRst.getToken(), "not valid call expression");
		break;
	}
	case lang::TypeKind::abstract: {
		if (calleeType.overloaded) {
			messageBag.bug(
			    callExprRst.getToken(),
			    std::format("overloaded functions not supported yet"));
			return;
		}
		break;
	}
	default: {
		messageBag.bug(callExprRst.getToken(), "unsupported type call");
		break;
	}
	}
	if (!calleeType.subtype.has_value()) {
		messageBag.bug(
		    callExprRst.getToken(),
		    std::format("expression does not have a return type for '{}'",
		                callExprRst.getToken().getLexeme()));
		return;
	}
	typeStack.push_back(*calleeType.subtype.value());
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
void TypeChecker::visitGroupingExpression(
    const syntax::rst::Grouping &groupingRST) {
	// the type of the grouping is just the child of the inner expression
	auto innerType = resolveType(*groupingRST.expression);

	if (innerType.has_value()) {
		typeStack.push_back(innerType.value());
	}
}
void TypeChecker::visitLiteralExpression(
    const syntax::rst::Literal &literalRST) {
	switch (literalRST.kind.type) {

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
		    literalRST.token.lexeme);
		if (!type.has_value()) {
			messageBag.error(
			    literalRST.getToken(),
			    std::format("'{}' cannot be hold in any scalar number type",
			                literalRST.getToken().getLexeme()));
			return;
		}
		typeStack.push_back(type.value());
		break;
	}
	case Token::TokenType::TOKEN_CHAR: {
		// any char token is a u8 character, not a unicode encode character
		// so only ASCII characters allowed
		const std::string_view character = literalRST.value;
		if (character.size() > 1) {
			messageBag.error(
			    literalRST.getToken(),
			    std::format("'{}' is not a valid char literal type",
			                literalRST.getToken().getLexeme()));
			break;
		}
		typeStack.push_back(
		    currentDataModel.get().findScalarType("u8").value());
		break;
	}
	default:
		messageBag.error(literalRST.getToken(),
		                 std::format("'{}' is not a valid literal type",
		                             literalRST.getToken().getLexeme()));
		break;
	}
}
void TypeChecker::visitLogicalExpression(const syntax::rst::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitSetExpression(const syntax::rst::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitUnaryExpression(
    const syntax::rst::Unary &unaryExpressionRST) {
	// TODO: assume that it returns the same type until we implement operator
	// overload
	// where we will treat each operator a a function

	auto innerType = resolveType(*unaryExpressionRST.expr);
	if (!innerType.has_value()) {
		messageBag.error(unaryExpressionRST.getToken(),
		                 "inner expression did not yield a type");
		return;
	}
	typeStack.push_back(innerType.value());
}
void TypeChecker::visitArrayAccessExpression(
    const syntax::rst::ArrayAccess &arrayAccessExpressionRST) {
	const auto accessedTypeR = resolveType(*arrayAccessExpressionRST.array);
	if (!accessedTypeR.has_value()) {
		messageBag.error(
		    arrayAccessExpressionRST.array->getToken(),
		    std::format(
		        "could not evaluate type for {}",
		        arrayAccessExpressionRST.array->getToken().getLexeme()));
		return;
	}
	const auto accessedType = accessedTypeR.value();
	// TODO: actually resolve with operator overload its return type
	if (!accessedType.subtype.has_value()) {
		messageBag.error(
		    arrayAccessExpressionRST.array->getToken(),
		    std::format(
		        "could not evaluate sub type for {}",
		        arrayAccessExpressionRST.array->getToken().getLexeme()));
		return;
	}
	const auto subType = accessedType.subtype.value();

	typeStack.push_back(*subType);
}
void TypeChecker::visitArrayTypeExpression(
    const syntax::rst::ArrayType &arrayTypeRST) {
	auto innerType = resolveType(*arrayTypeRST.subType)
	                     .value_or(lang::Type::defineUnknownType());
	if (innerType == lang::Type::defineUnknownType()) {
		messageBag.bug(arrayTypeRST.subType->getToken(),
		               std::format("inner array type is unknown for '{}'",
		                           arrayTypeRST.subType->getToken().lexeme));
		return;
	}
	if (innerType.getKind() == lang::TypeKind::abstract) {
		messageBag.error(arrayTypeRST.subType->getToken(),
		                 "arrays cannot hold abstract types");
		return;
	}
	lang::Type arrayType = currentDataModel.get().definePointerType(
	    innerType, arrayTypeRST.isMutable);
	typeStack.push_back(arrayType);
}
void TypeChecker::visitTupleTypeExpression(
    const syntax::rst::TupleType &tupleRST) {
	if (tupleRST.expressions.empty()) {
		auto unitType = currentDataModel.get().getUnitType();
		unitType.isMutable = tupleRST.isMutable;
		typeStack.push_back(unitType);
		return;
	}

	messageBag.bug(tupleRST.getToken(),
	               std::format("{} not implemented for non empty tuples",
	                           __PRETTY_FUNCTION__));
}
void TypeChecker::visitPointerTypeExpression(
    const syntax::rst::PointerType &pointerTypeRST) {
	auto subTypeResult = resolveType(*pointerTypeRST.subtype);
	if (!subTypeResult.has_value()) {
		messageBag.error(pointerTypeRST.token, "pointer subtype is unknown");
		return;
	}

	typeStack.push_back(currentDataModel.get().definePointerType(
	    subTypeResult.value(), pointerTypeRST.isMutable));
}
void TypeChecker::visitNamedTypeExpression(
    const syntax::rst::NamedType &typeRST) {
	auto result = findTypeInfo(typeRST.name.lexeme);
	if (result.has_value()) {
		lang::Type obtainedType = result.value();
		obtainedType.isMutable = typeRST.isMutable;
		typeStack.push_back(obtainedType);
	} else {
		messageBag.error(
		    typeRST.getToken(),
		    std::format("type not found for {}", typeRST.name.lexeme));
		typeStack.push_back(lang::Type::defineUnknownType());
	}
}
void TypeChecker::visitCastExpression(const syntax::rst::Cast &castRST) {
	// TODO: once operator overload is implemented add a cast for scalars
	// and make cast expression an overloaded type, or just make an into
	// trait if ever implemented
	auto type = resolveType(*castRST.type);
	if (!type.has_value()) {
		messageBag.error(
		    castRST.getToken(),
		    std::format("cast expression type '{}' did not yield a known type",
		                castRST.getToken().getLexeme()));
		return;
	}
	typeStack.push_back(type.value());
}
void TypeChecker::visitParameterExpression(
    const syntax::rst::Parameter &parameterRST) {
	const auto type = resolveType(*parameterRST.type.get());
	if (!type.has_value()) {
		messageBag.error(
		    parameterRST.getToken(),
		    std::format("parameter '{}' does not have a known type",
		                parameterRST.name.getLexeme()));
		return;
	}

	typeStack.push_back(type.value());
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

std::optional<lang::Type>
TypeChecker::findScalarTypeInfo(const std::string_view lexeme) {
	return currentDataModel.get().findScalarType(lexeme);
}
std::optional<lang::Type>
TypeChecker::findTypeInfo(const std::string_view typeName) {
	auto scalarType = findScalarTypeInfo(typeName);
	if (scalarType) {
		return scalarType;
	}
	// a defined type in the source unit cannot shadow a primitive/scalar
	// type
	auto foundStruct = currentSourceUnit.findStruct(typeName, currentScope);
	if (foundStruct.has_value()) {
		return currentDataModel.get().defineStructType(
		    foundStruct.value().get().structID, foundStruct.value().get().name,
		    0);
	}

	return std::nullopt;
}

std::optional<lang::FunctionDeclaration>
TypeChecker::resolveFunctionDeclaration(
    const syntax::rst::Function &functionRST) {
	std::optional<directive::LinkageDirective> linkageDirective;

	for (auto &directive : functionRST.compilerDirectives) {
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(),
			    std::format(
			        "unmatched compiler directive '{}' for function '{}'",
			        directive->directiveName(), functionRST.name.getLexeme()));
		}
	}
	std::string mangledFunctionName =
	    passes::mangling::NameMangler().mangleFunction(
	        currentSourceUnit.packageName, functionRST, linkageDirective);

	std::vector<lang::FunctionParameter> parameters;
	bool failed = false;
	for (const auto &parameter : functionRST.params) {
		auto paramType = resolveType(parameter);
		if (!paramType.has_value()) {
			messageBag.bug(
			    parameter.getToken(),
			    std::format("could not inspect type for {}",
			                parameter.type.get()->getToken().lexeme));
			failed = true;
			continue;
		}
		auto parameterType = paramType.value();
		if (parameterType.calculatedSize == 0) {
			messageBag.error(
			    parameter.type->getToken(),
			    std::format(
			        "cannot pass parameter type with unknown size for '{}'",
			        parameterType.name));
			failed = true;
			continue;
		}

		parameters.push_back({
		    .name = parameter.name.lexeme,
		    .parameterType = parameterType,
		});
	}

	auto functionReturnType =
	    functionRST.returnType
	        .transform([&](const auto &returnType) {
		        return resolveType(*returnType)
		            .value_or(lang::Type::defineUnknownType());
	        })
	        .value_or(lang::Type::defineUnknownType());

	auto returnType = functionReturnType;
	switch (returnType.getKind()) {

	case lang::TypeKind::abstract: {
		if (!returnType.coercercesInto(currentDataModel.get().getUnitType())) {
			failed = true;
			// TODO: review this in the future if we ever decide to return
			// abstract types at compile/evaluation time
			messageBag.error(
			    functionRST.returnType->get()->getToken(),
			    "abstract types cannot be returned from a function");
		}
		break;
	}
	case lang::TypeKind::scalar: {
		// scalar types do not need any type of checks
		// as they are fundamental types
		break;
	}
	case lang::TypeKind::aggregate: {
		// TODO: replace this for a known type checker
		if (returnType.calculatedSize == 0) {
			messageBag.error(
			    functionRST.returnType->get()->getToken(),
			    std::format("cannot return a type with unknown size for '{}'",
			                returnType.name));
			failed = true;
		}
		break;
	}
	case lang::TypeKind::pointer: {
		// pointer type was already evaluated and thus should be already a
		// known type
		break;
	}
	default: {
		failed = true;
		messageBag.bug(
		    functionRST.returnType->get()->getToken(),
		    std::format("unsupported return type for function with name '{}'",
		                returnType.name));
		break;
	}
	}

	if (failed) {
		return std::nullopt;
	}

	auto declaration = lang::FunctionDeclaration(
	    0, std::string(functionRST.name.getLexeme()), mangledFunctionName,
	    functionRST.publicVisibility,

	    lang::FunctionSignature{
	        .returnType = returnType,
	        .parameters = parameters,
	    });
	return declaration;
}

lang::Scope &TypeChecker::getCurrentScope() { return currentScope.get(); }
lang::Scope &TypeChecker::makeChildScope() {
	currentScope = currentScope.get().makeChildScope();
	return currentScope;
}
bool TypeChecker::popScope(lang::Scope &targetScope) {
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

} // namespace ray::compiler::passes::rst
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <format>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/typeChecker.hpp>
#include <ray/util/copy_ptr.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::passes {

void TypeChecker::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {

	for (const auto &stmt : statements) {
		auto stmtType = resolveType(*stmt);
		// we need to check the added types to the stack to see if they are
		// structs
		if (stmtType.has_value()) {
			auto type = stmtType.value();
			if (type.getKind() != lang::TypeKind::scalar) {
				// TODO: optimize this search
				// the types could have a reference to the specific struct
				/*


				*/
			} else {
				messageBag.bug(
				    stmt->getToken(),
				    std::format("unevaluated type value in stack for '{}'",
				                stmt->variantName()));
			}
		}
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

bool TypeChecker::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TypeChecker::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TypeChecker::getWarnings() const {
	return messageBag.getWarnings();
}

void TypeChecker::visitBlockStatement(const ast::Block &block) {
	std::vector<lang::Type> types;
	for (const auto &statement : block.statements) {
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
void TypeChecker::visitTerminalExprStatement(
    const ast::TerminalExpr &terminalExpr) {
	if (terminalExpr.expression.has_value()) {
		const auto &returnExpr = *terminalExpr.expression.value();
		auto returnType = resolveType(returnExpr);
		if (returnType.has_value()) {
			typeStack.push_back(returnType.value());
		} else {
			messageBag.error(
			    returnExpr.getToken(),
			    std::format("{} child expression did not yield a value '{}'",
			                terminalExpr.variantName(),
			                returnExpr.variantName()));
		}
		return;
	}

	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitExpressionStmtStatement(
    const ast::ExpressionStmt &exprStmt) {
	// an expression statement consumes the type and does not return a type
	// so it is an type of size 0 that cannot even be instatiated nor used
	resolveType(*exprStmt.expression);

	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitFunctionStatement(const ast::Function &functionExprAst) {

	auto declarationResult = resolveFunctionDeclaration(functionExprAst);
	if (!declarationResult.has_value()) {
		messageBag.error(
		    functionExprAst.getToken(),
		    std::format("could not resolve function declaration for '{}'",
		                functionExprAst.name.getLexeme()));
	} else {
		const auto functionDeclaration = declarationResult.value();
		if (!currentSourceUnit.declareFunction(functionDeclaration,
		                                       currentScope)) {
			messageBag.error(functionExprAst.getToken(),
			                 "could not declare function");
		}

		auto definition = lang::FunctionDefinition{
		    .declaration = functionDeclaration,
		    .function = functionExprAst,
		};
		std::vector<util::copy_ptr<lang::Type>> paramTypes;
		for (const auto &param : functionDeclaration.signature.parameters) {
			paramTypes.push_back(
			    util::copy_ptr<lang::Type>(param.parameterType));
		}

		// declaration was already defined, so it does not require to be defined
		// again, just the body
		if (functionExprAst.body.has_value()) {

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
					    functionExprAst.token,
					    std::format("parameter '{} 'could not be defined",
					                paramSymbol.name));
				}
			}
			const auto type =
			    resolveType(functionExprAst.body.value())
			        .value_or(currentDataModel.get().getUnitType());

			if (!type.coercercesInto(
			        functionDeclaration.signature.returnType)) {
				messageBag.error(
				    functionExprAst.body->getToken(),
				    std::format(
				        "inner body return type does not match with function return: '{}' vs '{}'",
				        type.name,
				        functionDeclaration.signature.returnType.name));
			}
		}

		auto functionType = currentDataModel.get().defineFunctionType(
		    functionDeclaration.signature.returnType, paramTypes);
		if (!currentSourceUnit.declareFunction(functionDeclaration,
		                                       getCurrentScope())) {
			messageBag.error(functionExprAst.getToken(),
			                 std::format("could not declare function for '{}'",
			                             functionDeclaration.name));
		}
		typeStack.push_back(functionType);
	}
}
void TypeChecker::visitIfStatement(const ast::If &ifStmt) {
	auto conditionType = resolveType(*ifStmt.condition);
	if (!conditionType.has_value()) {
		messageBag.error(ifStmt.condition->getToken(), "non boolean condition");
	} else {
		auto boolType = findScalarTypeInfo("bool");
		// for now lets just stricly validate if is the same
		// TODO: enable coercions
		if (!(conditionType->coercercesInto(boolType.value()))) {
			messageBag.error(ifStmt.condition->getToken(),
			                 "condition does not coerce into a bool type");
		}
	}
	auto thenRType = resolveType(*ifStmt.thenBranch);
	auto thenType = thenRType.has_value() ? thenRType.value()
	                                      : lang::Type::defineStmtType();
	if (ifStmt.elseBranch.has_value()) {
		auto elseRType = resolveType(*ifStmt.elseBranch.value());
		auto elseType = elseRType.has_value() ? elseRType.value()
		                                      : lang::Type::defineStmtType();
		// the types should match
		if (!(thenType == elseType)) {
			messageBag.error(
			    ifStmt.getToken(),
			    std::format("code branches have different types ({}|{})",
			                thenType.name, elseType.name));
		}
	}

	// return whatever our internal evaluation yield

	typeStack.push_back(thenType);
}
void TypeChecker::visitJumpStatement(const ast::Jump &jumpStmt) {
	// if our expression is a return we need to return its optional value
	// for anything else we do not care about its type
	if (jumpStmt.token.type == Token::TokenType::TOKEN_RETURN) {
		if (jumpStmt.returnValue.has_value()) {
			auto type = resolveType(*jumpStmt.returnValue.value());
			if (type.has_value()) {
				typeStack.push_back(type.value());
				return;
			}
		}
	}
	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitVarDeclStatement(const ast::VarDecl &variableDeclAst) {
	auto variableType = lang::Type{};

	if (variableDeclAst.type->getToken().type !=
	    Token::TokenType::TOKEN_UNINITIALIZED) {
		const auto &explicitType = variableDeclAst.type;
		std::string_view typeName = explicitType->getToken().lexeme;
		auto foundType = resolveType(*explicitType);
		if (!foundType.has_value()) {
			messageBag.error(
			    variableDeclAst.type->getToken(),
			    std::format("'{}' does not name an existing type", typeName));
		} else {
			variableType = foundType.value();
		}
	}

	if (variableDeclAst.initializer.has_value()) {
		auto &initializer = *variableDeclAst.initializer.value().get();
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
				    variableDeclAst.getToken(),
				    std::format(
				        "variable initialization type does not match with explicit type for '{}': '{}' vs '{}'",
				        variableDeclAst.getToken().getLexeme(),
				        variableType.name, initializationType.name));
			}
		}
	}

	if (variableType.isInitialized()) {
		lang::Symbol variableSymbol{
		    .name = variableDeclAst.name.lexeme,
		    .mangledName = variableDeclAst.name.lexeme,
		    .innerType = variableType,
		    .type = lang::Symbol::SymbolType::Parameter,
		    .internal = false,
		};

		if (!currentSourceUnit.declareLocalVariable(variableSymbol,
		                                            getCurrentScope())) {
			messageBag.bug(variableDeclAst.getToken(),
			               std::format("variable '{} 'could not be defined",
			                           variableSymbol.name));
		}
		// typeStack.push_back(variableType);
		return;
	}

	messageBag.error(
	    variableDeclAst.getToken(),
	    "variable does not have a valid type assigned nor an valid initialization");
}
void TypeChecker::visitMemberStatement(const ast::Member &variable) {
	auto variableType = lang::Type{};

	if (variable.type->getToken().type !=
	    Token::TokenType::TOKEN_UNINITIALIZED) {
		const auto &explicitType = variable.type;
		std::string_view typeName = explicitType->getToken().lexeme;
		auto foundType = resolveType(*explicitType);
		if (!foundType.has_value()) {
			messageBag.error(
			    variable.type->getToken(),
			    std::format("'{}' does not name an existing type", typeName));
		} else {
			variableType = foundType.value();
		}
	}

	if (variable.initializer.has_value()) {
		auto &initializer = *variable.initializer.value().get();
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
				    variable.getToken(),
				    std::format(
				        "member initialization type does not match with explicit type for '{}': '{}' vs '{}'",
				        variable.getToken().getLexeme(), variableType.name,
				        initializationType.name));
			}
		}
	}

	if (variableType.isInitialized()) {
		lang::Symbol variableSymbol{
		    .name = variable.name.lexeme,
		    .mangledName = "",
		    .innerType = variableType,
		    .type = lang::Symbol::SymbolType::Parameter,
		    .internal = false,
		};
		typeStack.push_back(variableType);
		return;
	}

	messageBag.error(
	    variable.getToken(),
	    "variable does not have a type assigned nor an valid initialization");
}
void TypeChecker::visitWhileStatement(const ast::While &whileStmt) {
	auto conditionType = resolveType(*whileStmt.condition);
	if (!conditionType.has_value()) {
		messageBag.error(whileStmt.condition->getToken(),
		                 "non boolean condition");
	} else {
		auto boolType = findScalarTypeInfo("bool");
		// for now lets just stricly validate if is the same
		// TODO: enable coercions
		if (!(conditionType->coercercesInto(boolType.value()))) {
			messageBag.error(whileStmt.condition->getToken(),
			                 "condition does not coerce into a bool type");
		}
	}

	const auto type =
	    resolveType(*whileStmt.body).value_or(lang::Type::defineStmtType());

	typeStack.push_back(type);
}
void TypeChecker::visitStructStatement(const ast::Struct &structObj) {
	// TODO: rework this section to just verify the existing struct
	return;
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
			    std::format("unmatched compiler directive '{}' for function.\n",
			                directive->directiveName()));
		}
		directivesStack.pop_back();
	}

	std::string structName = std::string(structObj.name.getLexeme());
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structObj,
	                                                 linkageDirective);

	if (!structObj.declaration) {
		std::vector<lang::StructMember> members;
		for (const auto &member : structObj.members) {
			auto memberType = resolveType(member);
			if (!memberType.has_value()) {
				messageBag.error(
				    member.getToken(),
				    std::format("could not get type information for '{}'",
				                member.name.lexeme));
				memberType = lang::Type::defineUnknownType();
			}

			auto newMember = lang::StructMember{
			    // for now set it as false, we will take care of it later
			    .publicVisibility = false,
			    .name = member.name.lexeme,
			    .type = memberType.value(),
			};
			members.push_back(newMember);
		}

		auto newStruct = lang::Struct{
		    .name = structName,
		    .mangledName = mangledStructName,
		    .members = members,
		};

		// TODO: remove declaration sections for type checker
		// if (currentSourceUnit.bindStruct(newStruct, currentScope.get())) {
		//	messageBag.error(structObj.getToken(),
		//	                 std::format("could not bind struct with name {}",
		//	                             structObj.name.getLexeme()));
		//}
	}

	size_t structId = 0;
	auto foundStruct = currentSourceUnit.findStruct(structName, currentScope);
	if (foundStruct.has_value()) {
		structId = foundStruct.value().get().structID;
	}
	auto structType =
	    currentDataModel.get().defineStructType(structId, structName, 1);
	typeStack.push_back(structType);
}
void TypeChecker::visitCompDirectiveStatement(
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
void TypeChecker::visitVariableExpression(const ast::Variable &variableExpr) {

	auto foundVariable =
	    getCurrentScope().findVariable(variableExpr.name.lexeme);
	if (foundVariable.has_value()) {
		typeStack.push_back(foundVariable.value().getObject()->get().innerType);
		return;
	}

	// we just keep a cound of at most 2 to see if we return its type pointer or
	// an overloadedFunction Type
	lang::Type functionType;
	for (const auto &functionDeclarationRef :
	     currentSourceUnit.findFunctionDeclarations(variableExpr.name.lexeme,
	                                                getCurrentScope())) {
		assert(functionDeclarationRef.getObject().has_value());
		const auto &functionDeclaration =
		    functionDeclarationRef.getObject()->get();
		if (!functionType.isInitialized()) {
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
	} else {
		messageBag.error(variableExpr.getToken(),
		                 std::format("unknown symbol '{}'",
		                             variableExpr.getToken().getLexeme()));
	}
	// we did not find anything so do not bother and report an error
	return;
}
void TypeChecker::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.bug(value.getToken(),
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitAssignExpression(const ast::Assign &assignExpr) {
	auto leftType = resolveType(*assignExpr.lhs);
	auto rightType = resolveType(*assignExpr.rhs);

	if (!(leftType.has_value() && rightType.has_value())) {
		if (!leftType.has_value()) {
			messageBag.error(
			    assignExpr.lhs->getToken(),
			    std::format("left expression did not yield a value"));
		}

		if (!rightType.has_value()) {
			messageBag.error(
			    assignExpr.rhs->getToken(),
			    std::format("right expression did not yield a value"));
			return;
		}
		return;
	}

	auto op = assignExpr.assignmentOp;
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
void TypeChecker::visitBinaryExpression(const ast::Binary &binaryExpr) {
	auto leftType = resolveType(*binaryExpr.left);
	auto rightType = resolveType(*binaryExpr.right);

	if (!(leftType.has_value() && rightType.has_value())) {
		if (!leftType.has_value()) {
			messageBag.error(
			    binaryExpr.left->getToken(),
			    std::format("left expression did not yield a value"));
		}

		if (!rightType.has_value()) {
			messageBag.error(
			    binaryExpr.right->getToken(),
			    std::format("right expression did not yield a value"));
		}
		return;
	}

	auto op = binaryExpr.op;
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
		messageBag.error(binaryExpr.op,
		                 std::format("'{}' is not a supported binary operation",
		                             op.getLexeme()));
	}
}
void TypeChecker::visitCallExpression(const ast::Call &callExpr) {
	auto calleeTypeResult = resolveType(*callExpr.callee);
	if (!calleeTypeResult.has_value()) {
		messageBag.error(callExpr.getToken(),
		                 std::format("unknown callee type for {}",
		                             callExpr.callee->getToken().getLexeme()));
		return;
	}
	auto calleeType = calleeTypeResult.value();

	switch (calleeType.getKind()) {

	case lang::TypeKind::pointer: {
		if (!calleeType.signature.has_value()) {
			messageBag.error(
			    callExpr.getToken(),
			    std::format(
			        "expression does not have a valid signature for '{}'",
			        callExpr.getToken().getLexeme()));
			break;
		}
		// valid signature
		if (callExpr.arguments.size() != calleeType.signature->size()) {
			messageBag.error(
			    callExpr.getToken(),
			    std::format(
			        "parameter number mismatch for '{}', provided {}, but {} were required",
			        callExpr.getToken().getLexeme(), callExpr.arguments.size(),
			        calleeType.signature->size()));
		}
		for (size_t i = 0; i < callExpr.arguments.size(); i++) {
			const auto &callerParamExpr = *callExpr.arguments[i];
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
				        "argument #'{}' does not matches the expected type {} vs {}",
				        i, callerParamType.name, calleeParamType.name));
				continue;
			}
		}

		break;
	}
	case lang::TypeKind::scalar:
	case lang::TypeKind::aggregate: {
		messageBag.error(callExpr.getToken(), "not valid call expression");
		break;
	}
	case lang::TypeKind::abstract: {
		if (calleeType.overloaded) {
			messageBag.bug(
			    callExpr.getToken(),
			    std::format("overloaded functions not supported yet"));
			return;
		}
		break;
	}
	default: {
		messageBag.bug(callExpr.getToken(), "unsupported type call");
		break;
	}
	}
	if (!calleeType.subtype.has_value()) {
		messageBag.bug(
		    callExpr.getToken(),
		    std::format("expression does not have a return type for '{}'",
		                callExpr.getToken().getLexeme()));
		return;
	}
	typeStack.push_back(*calleeType.subtype.value());
}
void TypeChecker::visitIntrinsicCallExpression(
    const ast::IntrinsicCall &intrinsicCall) {

	switch (intrinsicCall.callee->intrinsic) {
	case ray::compiler::ast::IntrinsicType::INTR_SIZEOF: {
		if (intrinsicCall.arguments.size() != 1) {
			messageBag.error(
			    intrinsicCall.callee->name,
			    std::format(
			        "{} intrinsic expects 1 argument but {} got provided",
			        intrinsicCall.callee->name.lexeme,
			        intrinsicCall.arguments.size()));
		} else {
			auto param = intrinsicCall.arguments[0].get();
			auto paramType = resolveType(*param);
			if (paramType.has_value()) {
				// TODO: check the type and then return the size type
			}
		}
		break;
	}
	case ray::compiler::ast::IntrinsicType::INTR_IMPORT: {
		if (intrinsicCall.arguments.size() != 1) {
			messageBag.error(intrinsicCall.callee->name,
			                 std::format("{} intrinsic expects 1 "
			                             "argument but {} got provided",
			                             intrinsicCall.callee->name.lexeme,
			                             intrinsicCall.arguments.size()));
		} else {
			auto moduleType = lang::Type::defineModuleType();
			// TODO: create a module provided to idenity known module data
			typeStack.push_back(moduleType);
		}

		break;
	}
	case ray::compiler::ast::IntrinsicType::INTR_UNKNOWN:
		messageBag.error(intrinsicCall.callee->name,
		                 std::format("'{}' is not a valid intrinsic",
		                             intrinsicCall.callee->name.lexeme));
		break;
	}
}
void TypeChecker::visitGetExpression(const ast::Get &getExpression) {}
void TypeChecker::visitGroupingExpression(const ast::Grouping &groupingExpr) {
	// the type of the grouping is just the child of the inner expression
	auto innerType = resolveType(*groupingExpr.expression);

	if (innerType.has_value()) {
		typeStack.push_back(innerType.value());
	}
}
void TypeChecker::visitLiteralExpression(const ast::Literal &literalExpr) {
	switch (literalExpr.kind.type) {

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
		    literalExpr.token.lexeme);
		if (!type.has_value()) {
			messageBag.error(
			    literalExpr.getToken(),
			    std::format("'{}' cannot be hold in any scalar number type",
			                literalExpr.getToken().getLexeme()));
			return;
		}
		typeStack.push_back(type.value());
		break;
	}
	case Token::TokenType::TOKEN_CHAR: {
		// any char token is a u8 character, not a unicode encode character
		// so only ASCII characters allowed
		const std::string_view character = literalExpr.value;
		if (character.size() > 1) {
			messageBag.error(
			    literalExpr.getToken(),
			    std::format("'{}' is not a valid char literal type",
			                literalExpr.getToken().getLexeme()));
			break;
		}
		typeStack.push_back(
		    currentDataModel.get().findScalarType("u8").value());
		break;
	}
	default:
		messageBag.error(literalExpr.getToken(),
		                 std::format("'{}' is not a valid literal type",
		                             literalExpr.getToken().getLexeme()));
		break;
	}
}
void TypeChecker::visitLogicalExpression(const ast::Logical &logicalExpr) {
	auto leftType = resolveType(*logicalExpr.left);
	auto rightType = resolveType(*logicalExpr.right);

	if (!(leftType.has_value() && rightType.has_value())) {
		if (!leftType.has_value()) {
			messageBag.error(
			    logicalExpr.left->getToken(),
			    std::format("left expression did not yield a value"));
		}

		if (!rightType.has_value()) {
			messageBag.error(
			    logicalExpr.right->getToken(),
			    std::format("right expression did not yield a value"));
		}
		return;
	}

	auto op = logicalExpr.op;
	// TODO: once we start supporting operator overload this should be done by
	// lookup of the overloads and get the return type of it
	switch (op.type) {

	case Token::TokenType::TOKEN_AMPERSAND_AMPERSAND:
	case Token::TokenType::TOKEN_PIPE_PIPE:
		typeStack.push_back(findScalarTypeInfo("bool").value());
		break;
	default:
		messageBag.error(
		    logicalExpr.op,
		    std::format("'{}' is not a supported logical operation",
		                op.getLexeme()));
	}
}
void TypeChecker::visitSetExpression(const ast::Set &value) {
	messageBag.bug(value.getToken(),
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitUnaryExpression(const ast::Unary &unaryExpr) {
	// assume that it returns the same type until we implement operator overload
	// where we will treat each operator a a function

	auto innerType = resolveType(*unaryExpr.expr);
	if (!innerType.has_value()) {
		messageBag.error(unaryExpr.getToken(),
		                 "inner expression did not yield a type");
		return;
	}
	typeStack.push_back(innerType.value());
}
void TypeChecker::visitArrayAccessExpression(
    const ast::ArrayAccess &arrayExpr) {
	const auto accessedTypeR = resolveType(*arrayExpr.array);
	if (!accessedTypeR.has_value()) {
		messageBag.error(arrayExpr.array->getToken(),
		                 std::format("could not evaluate type for {}",
		                             arrayExpr.array->getToken().getLexeme()));
		return;
	}
	const auto accessedType = accessedTypeR.value();
	// TODO: actually resolve with operator overload its return type
	if (!accessedType.subtype.has_value()) {
		messageBag.error(arrayExpr.array->getToken(),
		                 std::format("could not evaluate sub type for {}",
		                             arrayExpr.array->getToken().getLexeme()));
		return;
	}
	const auto subType = accessedType.subtype.value();

	typeStack.push_back(*subType);
}
void TypeChecker::visitArrayTypeExpression(const ast::ArrayType &value) {
	messageBag.bug(value.getToken(),
	               std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitTupleTypeExpression(const ast::TupleType &value) {
	messageBag.bug(value.getToken(),
	               std::format("{} not implemented", __PRETTY_FUNCTION__));
}

void TypeChecker::visitPointerTypeExpression(
    const ast::PointerType &pointerTypeAst) {
	auto subTypeResult = resolveType(*pointerTypeAst.subtype);
	if (!subTypeResult.has_value()) {
		messageBag.error(pointerTypeAst.token, "pointer subtype is unknown");
		return;
	}

	typeStack.push_back(currentDataModel.get().definePointerType(
	    subTypeResult.value(), pointerTypeAst.isMutable));
}
void TypeChecker::visitNamedTypeExpression(const ast::NamedType &typeAst) {
	auto result = findTypeInfo(typeAst.name.lexeme);
	if (result.has_value()) {
		lang::Type obtainedType = result.value();
		obtainedType.isMutable = typeAst.isMutable;
		typeStack.push_back(obtainedType);
	} else {
		messageBag.error(
		    typeAst.getToken(),
		    std::format("type not found for {}", typeAst.name.lexeme));
		typeStack.push_back(lang::Type::defineUnknownType());
	}
}
void TypeChecker::visitCastExpression(const ast::Cast &castExpr) {
	// TODO: remove cast expression and make it a compiler intrinsic for scalars
	// additionally make the propper checks, for now we just blindly cast the
	// expression
	auto type = resolveType(*castExpr.type);
	if (!type.has_value()) {
		messageBag.error(
		    castExpr.getToken(),
		    std::format("cast expression type '{}' did not yield a known type",
		                castExpr.getToken().getLexeme()));
		return;
	}
	typeStack.push_back(type.value());
}
void TypeChecker::visitParameterExpression(const ast::Parameter &parameter) {
	const auto type = resolveType(*parameter.type.get());
	if (!type.has_value()) {
		messageBag.error(
		    parameter.getToken(),
		    std::format("parameter '{}' does not have a known type",
		                parameter.name.getLexeme()));
		return;
	}

	typeStack.push_back(type.value());
}

std::optional<lang::Type>
TypeChecker::resolveType(const ast::Statement &statement) {
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
TypeChecker::resolveType(const ast::Expression &expression) {
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
TypeChecker::resolveTypes(const ast::Statement &statement) {
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
TypeChecker::resolveTypes(const ast::Expression &expression) {
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
	// a defined type in the source unit cannot shadow a primitive/scalar type
	auto foundStruct = currentSourceUnit.findStruct(typeName, currentScope);
	if (foundStruct.has_value()) {
		return currentDataModel.get().defineStructType(
		    foundStruct.value().get().structID, foundStruct.value().get().name,
		    0);
	}

	return std::nullopt;
}

std::optional<lang::FunctionDeclaration>
TypeChecker::resolveFunctionDeclaration(const ast::Function &functionAst) {
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

	std::vector<lang::FunctionParameter> parameters;
	bool failed = false;
	for (const auto &parameter : functionAst.params) {
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

	auto functionReturnType = resolveType(*functionAst.returnType);
	if (!functionReturnType.has_value()) {
		return std::nullopt;
	}
	auto returnType = functionReturnType.value();
	// TODO: make this use the type kind instead for checks of the size
	if ((returnType.getKind() != lang::TypeKind::scalar) &&
	    returnType.calculatedSize == 0) {
		messageBag.error(
		    functionAst.returnType->getToken(),
		    std::format("cannot return a type with unknown size for '{}'",
		                returnType.name));
		failed = true;
	}

	if (failed) {
		return std::nullopt;
	}

	auto declaration = lang::FunctionDeclaration{
	    .name = std::string(functionAst.name.getLexeme()),
	    .mangledName = mangledFunctionName,
	    .publicVisibility = functionAst.publicVisibility,
	    .signature =
	        lang::FunctionSignature{
	            .returnType = returnType,
	            .parameters = parameters,
	        },
	};
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

} // namespace ray::compiler::passes
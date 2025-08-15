#include "ray/compiler/lang/symbol.hpp"
#include <cstddef>
#include <cstdio>
#include <format>
#include <optional>
#include <string_view>
#include <vector>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/typeChecker.hpp>
#include <ray/util/copy_ptr.hpp>

namespace ray::compiler::analyzer {

void TypeChecker::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {

	for (const auto &stmt : statements) {
		if (const auto functionExpr =
		        dynamic_cast<const ast::Function *>(stmt.get())) {

			auto parameters = std::vector<lang::FunctionParameter>{};

			auto declaration = resolveFunctionDeclaration(*functionExpr);
			if (declaration.has_value()) {
				currentSourceUnit.functionDeclarations.push_back(
				    declaration.value());
			}
		}
	}

	for (const auto &stmt : statements) {
		auto stmtType = resolveType(*stmt);
		// we need to check the added types to the stack to see if they are
		// structs
		if (stmtType.has_value()) {
			auto type = stmtType.value();
			if (!type.isScalar()) {
				// TODO: optimize this search
				// the types could have a reference to the specific struct
				for (const auto &structDeclaration :
				     currentSourceUnit.structDeclarations) {
					if (structDeclaration.name == type.name) {
						if (!currentScope.get().defineStruct(type)) {
							messageBag.error(
							    stmt->getToken(), "TYPE-CHECKER",
							    std::format("cannot redefine type '{}'",
							                type.name));
						}
					}
				}
			} else {
				messageBag.bug(
				    stmt->getToken(), "TYPE-CHECKER",
				    std::format("unevaluated type value in stack for '{}'",
				                stmt->variantName()));
			}
		}
	}

	for (auto &directive : directivesStack) {
		messageBag.warning(directive->getToken(), "TypeChecker",
		                   std::format("unused compiler directive {}",
		                               directive->directiveName()));
	}

	if (typeStack.size() > 0) {
		Token errorToken{Token::TokenType::TOKEN_EOF,
		                 std::string(Token::glyph(Token::TokenType::TOKEN_EOF)),
		                 0, 0};
		messageBag.bug(errorToken, "TYPE-CHECKER",
		               std::format("type stack evaluation error"));
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
			    returnExpr.getToken(), "TYPE-CHECKER",
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
void TypeChecker::visitFunctionStatement(const ast::Function &function) {

	auto declarationResult = resolveFunctionDeclaration(function);
	if (!declarationResult.has_value()) {
		messageBag.error(
		    function.getToken(), "TYPE-CHECKER",
		    std::format("could not resolve function declaration for '{}'",
		                function.name.getLexeme()));
	} else {
		const auto declaration = declarationResult.value();
		auto definition = lang::FunctionDefinition{
		    .declaration = declaration,
		    .function = function,
		};
		std::vector<util::copy_ptr<lang::Type>> paramTypes;
		for (const auto &param : declaration.signature.parameters) {
			paramTypes.push_back(
			    util::copy_ptr<lang::Type>(param.parameterType));
		}

		// declaration was already defined, so it does not require to be defined
		// again, just the body
		if (function.body.has_value()) {
			currentSourceUnit.functionDefinitions.push_back(definition);

			// add functions to the current scope and validate that each
			for (const auto &param : declaration.signature.parameters) {
				// TODO: variable definitions should be done at an earlier stage
				lang::Symbol paramSymbol{
				    .name = param.name,
				    .mangledName = "",
				    .innerType = param.parameterType,
				    .type = lang::Symbol::SymbolType::Parameter,
				    .internal = false,
				};
				currentScope.get().defineLocalVariable(paramSymbol);
			}
			const auto type = resolveType(function.body.value())
			                      .value_or(lang::Type::getVoidType());

			if (!type.coercercesInto(declaration.signature.returnType)) {
				messageBag.error(
				    function.body->getToken(), "TYPE-CHECKER",
				    std::format(
				        "inner body return type does not match with function return: '{}' vs '{}'",
				        type.name, declaration.signature.returnType.name));
			}
		}

		auto functionType = lang::Type::defineFunctionType(
		    declaration.signature.returnType, paramTypes);
		if (!currentScope.get().defineFunction(declaration)) {
			messageBag.error(function.getToken(), "TYPE-CHECKER",
			                 std::format("could not declare function for '{}'",
			                             declaration.name));
		}
		typeStack.push_back(functionType);
	}
}
void TypeChecker::visitIfStatement(const ast::If &ifStmt) {
	auto conditionType = resolveType(*ifStmt.condition);
	if (!conditionType.has_value()) {
		messageBag.error(ifStmt.condition->getToken(), "TYPE-CHECKER",
		                 "non boolean condition");
	} else {
		auto boolType = findScalarTypeInfo("bool");
		// for now lets just stricly validate if is the same
		// TODO: enable coercions
		if (!(conditionType->coercercesInto(boolType.value()))) {
			messageBag.error(ifStmt.condition->getToken(), "TYPE-CHECKER",
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
			    ifStmt.getToken(), "TYPE-CHECKER",
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
		if (jumpStmt.value.has_value()) {
			auto type = resolveType(*jumpStmt.value.value());
			if (type.has_value()) {
				typeStack.push_back(type.value());
				return;
			}
		}
	}
	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitVarStatement(const ast::Var &variable) {
	auto variableType = lang::Type{};

	if (variable.type.token.type != Token::TokenType::TOKEN_UNINITIALIZED) {
		const auto &explicitType = variable.type;
		std::string_view typeName = explicitType.name.lexeme;
		auto foundType = resolveType(explicitType);
		if (!foundType.has_value()) {
			messageBag.error(
			    variable.type.getToken(), "TYPE-CHECKER",
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
			    initializer.getToken(), "TYPE-CHECKER",
			    std::format(
			        "inialization expression did not yield a type for '{}'",
			        initializer.getToken().getLexeme()));
		} else {
			const auto initializationType = initType.value();
			if (!variableType.isInitialized()) {
				variableType = initializationType;
			} else if (!initializationType.coercercesInto(variableType)) {
				messageBag.error(
				    variable.getToken(), "TYPE-CHECKER",
				    std::format(
				        "variable initialization type does not match with explicit type for '{}': '{}' vs '{}'",
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
		currentScope.get().defineLocalVariable(variableSymbol);
		// typeStack.push_back(variableType);
		return;
	}

	messageBag.error(
	    variable.getToken(), "TYPE-CHECKER",
	    "variable does not have a type assigned nor an valid initialization");
}
void TypeChecker::visitWhileStatement(const ast::While &whileStmt) {
	auto conditionType = resolveType(*whileStmt.condition);
	if (!conditionType.has_value()) {
		messageBag.error(whileStmt.condition->getToken(), "TYPE-CHECKER",
		                 "non boolean condition");
	} else {
		auto boolType = findScalarTypeInfo("bool");
		// for now lets just stricly validate if is the same
		// TODO: enable coercions
		if (!(conditionType->coercercesInto(boolType.value()))) {
			messageBag.error(whileStmt.condition->getToken(), "TYPE-CHECKER",
			                 "condition does not coerce into a bool type");
		}
	}

	const auto type =
	    resolveType(*whileStmt.body).value_or(lang::Type::defineStmtType());

	typeStack.push_back(type);
}
void TypeChecker::visitStructStatement(const ast::Struct &structObj) {
	std::string currentModule;

	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > top; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(), "TYPE-CHECKER",
			    std::format("unmatched compiler directive '{}' for function.\n",
			                directive->directiveName()));
		}
		directivesStack.pop_back();
	}

	std::string structName = std::string(structObj.name.getLexeme());
	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structObj,
	                                                 linkageDirective);

	size_t structSize = 0;
	bool platformDependent = false;

	if (!structObj.declaration) {
		std::vector<lang::StructMember> members;
		for (const auto &member : structObj.members) {
			auto memberType = resolveType(member);
			if (!memberType.has_value()) {
				messageBag.error(
				    member.getToken(), "TYPE-CHECKER",
				    std::format("could not get type information for '{}'",
				                member.name.lexeme));
				return;
			}

			auto newMember = lang::StructMember{
			    // for now set it as false, we will take care of it later
			    .publicAccess = false,
			    .name = member.name.lexeme,
			    .type = memberType.value(),
			};
			members.push_back(newMember);
		}

		auto newStruct = lang::Struct{
		    .name = structName,
		    .mangledName = mangledStructName,
		    .members = members,
		    .structObj = structObj,
		};
		if (structSize != 0) {
			// push it to the type stack the top evaluator is responsible to
			// define
			// it at its own level and potentially mangle its name again
			currentSourceUnit.structDefinitions.push_back(newStruct);
		}
	}

	auto structType =
	    lang::Type::defineStructType(structName, structSize, platformDependent);
	currentSourceUnit.structDeclarations.push_back(lang::StructDeclaration{
	    .name = structName,
	    .mangledName = mangledStructName,
	});
	typeStack.push_back(structType);
}
void TypeChecker::visitCompDirectiveStatement(
    const ast::CompDirective &compDirective) {
	auto directiveToken = compDirective.name;
	auto directiveName = compDirective.name.getLexeme();
	if (directiveName == "Linkage") {
		auto &attributes = compDirective.values;
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
		if (compDirective.child) {
			auto childValue = compDirective.child.get();
			if (dynamic_cast<ast::Function *>(childValue) ||
			    dynamic_cast<ast::Struct *>(childValue)) {
				size_t startDirectives = directivesStack.size();
				size_t originalTop = top + 1;
				top = startDirectives;
				directivesStack.push_back(
				    std::make_unique<directive::LinkageDirective>(directive));
				auto directiveType = resolveType(*compDirective.child);
				if (directiveType.has_value()) {
					typeStack.push_back(directiveType.value());
				}
				if (directivesStack.size() != startDirectives) {
					messageBag.bug(childValue->getToken(), "TYPE-CHECKER",
					               "unprocessed compiler directives");
				}

				top = originalTop;
			} else {
				messageBag.error(
				    childValue->getToken(), "TYPE-CHECKER",
				    std::format(
				        "{} child expression must be a function or a struct.",
				        directive.directiveName()));
			}
		} else {
			messageBag.error(compDirective.getToken(), "TYPE-CHECKER",
			                 std::format("{} must have a child expression.",
			                             directive.directiveName()));
		}
	} else {
		messageBag.error(
		    compDirective.getToken(), "TYPE-CHECKER",
		    std::format("Unknown compiler directive '{}'.", directiveName));
	}
}
// Expression
void TypeChecker::visitVariableExpression(const ast::Variable &variableExpr) {

	if (currentScope.get().variables.contains(variableExpr.name.lexeme)) {
		auto type = currentScope.get().variables.at(variableExpr.name.lexeme);
		typeStack.push_back(type.innerType);
		return;
	}

	// we just keep a cound of at most 2 to see if we return its type pointer or
	// an overloadedFunction Type
	lang::Type functionType;
	for (const auto &functionDeclaration :
	     currentSourceUnit.functionDeclarations) {
		if (functionDeclaration.name == variableExpr.name.lexeme) {
			if (!functionType.isInitialized()) {
				functionType = functionDeclaration.signature.getFunctionType();
			} else {
				functionType =
				    functionDeclaration.signature.getOverloadedFunctionType();
				break;
			}
		}
	}

	if (functionType.isInitialized()) {
		typeStack.push_back(functionType);
	} else {
		messageBag.error(variableExpr.getToken(), "TYPE-CHECKER",
		                 std::format("unknown symbol '{}'",
		                             variableExpr.getToken().getLexeme()));
	}
	// we did not find anything so do not bother and report an error
	return;
}
void TypeChecker::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitAssignExpression(const ast::Assign &assignExpr) {
	auto leftType = resolveType(*assignExpr.lhs);
	auto rightType = resolveType(*assignExpr.rhs);

	if (!(leftType.has_value() && rightType.has_value())) {
		if (!leftType.has_value()) {
			messageBag.error(
			    assignExpr.lhs->getToken(), "TYPE-CHECKER",
			    std::format("left expression did not yield a value"));
		}

		if (!rightType.has_value()) {
			messageBag.error(
			    assignExpr.rhs->getToken(), "TYPE-CHECKER",
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
		    op, "TYPE-CHECKER",
		    std::format("'{}' is not a supported assignment operation",
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
			    binaryExpr.left->getToken(), "TYPE-CHECKER",
			    std::format("left expression did not yield a value"));
		}

		if (!rightType.has_value()) {
			messageBag.error(
			    binaryExpr.right->getToken(), "TYPE-CHECKER",
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
		messageBag.error(binaryExpr.op, "TYPE-CHECKER",
		                 std::format("'{}' is not a supported binary operation",
		                             op.getLexeme()));
	}
}
void TypeChecker::visitCallExpression(const ast::Call &callExpr) {
	auto calleeTypeResult = resolveType(*callExpr.callee);
	if (!calleeTypeResult.has_value()) {
		messageBag.error(callExpr.getToken(), "TYPE-CHECKER",
		                 std::format("unknown callee type for {}",
		                             callExpr.callee->getToken().getLexeme()));
		return;
	}
	auto calleeType = calleeTypeResult.value();
	if (calleeType.overloaded) {
		messageBag.bug(callExpr.getToken(), "TYPE-CHECKER",
		               std::format("overloaded functions not supported yet"));
		return;
	}
	if (!calleeType.signature.has_value()) {
		messageBag.error(
		    callExpr.getToken(), "TYPE-CHECKER",
		    std::format("expression does not have a valid signature for '{}'",
		                callExpr.getToken().getLexeme()));
		return;
	} else {
		// non overloaded, so we need to validate the parameters
		if (callExpr.arguments.size() != calleeType.signature->size()) {
			messageBag.error(
			    callExpr.getToken(), "TYPE-CHECKER",
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
				    callerParamExpr.getToken(), "TYPE-CHECKER",
				    std::format("argument does not yield a valid type for '{}'",
				                callerParamExpr.getToken().getLexeme()));
				return;
			}
			const auto &callerParamType = callerParamTypeResult.value();

			if (!callerParamType.coercercesInto(calleeParamType)) {
				messageBag.error(
				    callerParamExpr.getToken(), "TYPE-CHECKER",
				    std::format(
				        "argument #'{}' does not matches the expected type {} vs {}",
				        i, callerParamType.name, calleeParamType.name));
				continue;
			}
		}
	}

	if (!calleeType.subtype.has_value()) {
		messageBag.bug(
		    callExpr.getToken(), "TYPE-CHECKER",
		    std::format("expression does not have a return type for '{}'",
		                callExpr.getToken().getLexeme()));
	}
	typeStack.push_back(*calleeType.subtype.value());
}
void TypeChecker::visitIntrinsicCallExpression(
    const ast::IntrinsicCall &intrinsicCall) {

	switch (intrinsicCall.callee->intrinsic) {
	case ray::compiler::ast::IntrinsicType::INTR_SIZEOF: {
		if (intrinsicCall.arguments.size() != 1) {
			messageBag.error(
			    intrinsicCall.callee->name, "TYPE-CHECKER",
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
			messageBag.error(intrinsicCall.callee->name, "TYPE-CHECKER",
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
		messageBag.error(intrinsicCall.callee->name, "TYPE-CHECKER",
		                 std::format("'{}' is not a valid intrinsic",
		                             intrinsicCall.callee->name.lexeme));
		break;
	}
}
void TypeChecker::visitGetExpression(const ast::Get &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
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
		const auto baseType = lang::Type::findScalarType("u8").value();
		const auto arrayType = makePointerType(baseType);
		typeStack.push_back(arrayType);
		break;
	}
	case Token::TokenType::TOKEN_NUMBER: {
		auto type = lang::Type::getNumberLiteralType(literalExpr.token.lexeme);
		if (!type.has_value()) {
			messageBag.error(
			    literalExpr.getToken(), "TYPE-CHECKER",
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
			    literalExpr.getToken(), "TYPE-CHECKER",
			    std::format("'{}' is not a valid char literal type",
			                literalExpr.getToken().getLexeme()));
			break;
		}
		typeStack.push_back(lang::Type::findScalarType("u8").value());
		break;
	}
	default:
		messageBag.error(literalExpr.getToken(), "TYPE-CHECKER",
		                 std::format("'{}' is not a valid literal type",
		                             literalExpr.getToken().getLexeme()));
		break;
	}
}
void TypeChecker::visitLogicalExpression(const ast::Logical &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitSetExpression(const ast::Set &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitUnaryExpression(const ast::Unary &unaryExpr) {
	// assume that it returns the same type until we implement operator overload
	// where we will treat each operator a a function

	auto innerType = resolveType(*unaryExpr.expr);
	if (!innerType.has_value()) {
		messageBag.error(unaryExpr.getToken(), "TYPE-CHECKER",
		                 "inner expression did not yield a type");
		return;
	}
	typeStack.push_back(innerType.value());
}
void TypeChecker::visitArrayAccessExpression(
    const ast::ArrayAccess &arrayExpr) {
	const auto accessedTypeR = resolveType(*arrayExpr.array);
	if (!accessedTypeR.has_value()) {
		messageBag.error(arrayExpr.array->getToken(), "TYPE-CHECKER",
		                 std::format("could not evaluate type for {}",
		                             arrayExpr.array->getToken().getLexeme()));
		return;
	}
	const auto accessedType = accessedTypeR.value();
	// TODO: actually resolve with operator overload its return type
	if (!accessedType.subtype.has_value()) {
		messageBag.error(arrayExpr.array->getToken(), "TYPE-CHECKER",
		                 std::format("could not evaluate sub type for {}",
		                             arrayExpr.array->getToken().getLexeme()));
		return;
	}
	const auto subType = accessedType.subtype.value();

	typeStack.push_back(*subType);
}
void TypeChecker::visitTypeExpression(const ast::Type &typeExpr) {
	if (typeExpr.isPointer) {
		auto innerType = resolveType(*typeExpr.subtype.value());
		if (!innerType.has_value()) {
			messageBag.error(typeExpr.getToken(), "TYPE-CHECKER",
			                 std::format("could not evaluate type for {}",
			                             typeExpr.getToken().getLexeme()));
			return;
		}
		lang::Type pointerType = makePointerType(innerType.value());
		pointerType.isMutable = typeExpr.isMutable;
		typeStack.push_back(pointerType);
	} else {
		auto result = findTypeInfo(typeExpr.name.lexeme);
		if (result.has_value()) {
			lang::Type obtainedType = result.value();
			obtainedType.isMutable = typeExpr.isMutable;
			typeStack.push_back(obtainedType);
		} else {
			messageBag.error(
			    typeExpr.getToken(), "TYPE-CHECKER",
			    std::format("type not found for {}", typeExpr.name.lexeme));
		}
	}
}
void TypeChecker::visitCastExpression(const ast::Cast &castExpr) {
	// TODO: remove cast expression and make it a compiler intrinsic for scalars
	// additionally make the propper checks, for now we just blindly cast the
	// expression
	auto type = resolveType(castExpr.type);
	if (!type.has_value()) {
		messageBag.error(
		    castExpr.getToken(), "TYPE-CHECKER",
		    std::format("cast expression type '{}' did not yield a known type",
		                castExpr.getToken().getLexeme()));
		return;
	}
	typeStack.push_back(type.value());
}
void TypeChecker::visitParameterExpression(const ast::Parameter &parameter) {
	const auto type = resolveType(parameter.type);
	if (!type.has_value()) {
		messageBag.error(
		    parameter.getToken(), "TYPE-CHECKER",
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
				    statement.getToken(), "TYPE-CHECKER",
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
		messageBag.bug(expression.getToken(), "TYPE-CHECKER",
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
		messageBag.bug(expression.getToken(), "TYPE-CHECKER",
		               std::format("'{}' did not yield any return type",
		                           expression.variantName()));
	}
	return returnTypes;
}

std::optional<lang::Type>
TypeChecker::findScalarTypeInfo(const std::string_view lexeme) {
	return lang::Type::findScalarType(lexeme);
}
std::optional<lang::Type>
TypeChecker::findTypeInfo(const std::string_view typeName) {
	auto scalarType = findScalarTypeInfo(typeName);
	if (scalarType) {
		return scalarType;
	}
	// a defined type in the source unit cannot shadow a primitive/scalar type
	return currentSourceUnit.findStructType(std::string(typeName));
}

lang::Type TypeChecker::makePointerType(const lang::Type &innerType) {
	// TODO: move this to a separate code section
	return lang::Type(true,
	                  // a pointer is not a scalar as it is an address memory
	                  // that references an object
	                  false,
	                  // technically platform dependent on pointer definition
	                  true,
	                  // define the name as pointer
	                  "%<pointer>%",
	                  // we need to get this from the platform in the future
	                  // for now assuming 64bits/8bytes
	                  8,
	                  // if the pointer type is const or not is decided later
	                  true,
	                  // we are a pointer type
	                  true,        // pointer
	                  false,       // non signed
	                  false,       // non overloaded
	                  {innerType}, // inner type
	                  {}           // no signature
	);
}

std::optional<lang::FunctionDeclaration>
TypeChecker::resolveFunctionDeclaration(const ast::Function &function) {
	std::string currentModule;

	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > top; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(), "TYPE-CHECKER",
			    std::format(
			        "unmatched compiler directive '{}' for function '{}'",
			        directive->directiveName(), function.name.getLexeme()));
		}
		directivesStack.pop_back();
	}
	std::string mangledFunctionName =
	    passes::mangling::NameMangler().mangleFunction(currentModule, function,
	                                                   linkageDirective);

	std::vector<lang::FunctionParameter> parameters;
	bool failed = false;
	for (const auto &parameter : function.params) {
		auto paramType = resolveType(parameter);
		if (!paramType.has_value()) {
			messageBag.bug(parameter.getToken(), "TYPE-CHECKER",
			               std::format("could not inspect type for {}",
			                           parameter.type.name.lexeme));
			failed = true;
			continue;
		}
		auto parameterType = paramType.value();
		if (parameterType.calculatedSize == 0) {
			messageBag.error(
			    parameter.type.getToken(), "TYPE-CHECKER",
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

	auto functionReturnType = resolveType(function.returnType);
	if (!functionReturnType.has_value()) {
		return std::nullopt;
	}
	auto returnType = functionReturnType.value();
	if (!returnType.isScalar() && returnType.calculatedSize == 0) {
		messageBag.error(
		    function.returnType.getToken(), "TYPE-CHECKER",
		    std::format("cannot return a type with unknown size for '{}'",
		                returnType.name));
		failed = true;
	}

	if (failed) {
		return std::nullopt;
	}

	auto declaration = lang::FunctionDeclaration{
	    .name = std::string(function.name.getLexeme()),
	    .mangledName = mangledFunctionName,
	    .publicVisibility = function.publicVisibility,
	    .signature =
	        lang::FunctionSignature{
	            .returnType = returnType,
	            .parameters = parameters,
	        },
	};
	return declaration;
}

} // namespace ray::compiler::analyzer
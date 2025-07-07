#include <cstddef>
#include <cstdio>
#include <format>
#include <optional>
#include <sstream>
#include <vector>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/typeChecker.hpp>

namespace ray::compiler::analyzer {

void TypeChecker::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statements) {
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
		types.insert(types.end(), stmtTypes.begin(), stmtTypes.end());
	}

	typeStack.reserve(typeStack.size() + types.size());
	typeStack.insert(typeStack.end(), types.begin(), types.end());
}
void TypeChecker::visitTerminalExprStatement(const ast::TerminalExpr &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitExpressionStmtStatement(
    const ast::ExpressionStmt &exprStmt) {
	// an expression statement consumes the type and does not return a type
	// so it is an type of size 0 that cannot even be instatiated nor used
	resolveType(*exprStmt.expression);

	typeStack.push_back(lang::Type::defineStmtType());
}
void TypeChecker::visitFunctionStatement(const ast::Function &function) {
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
	for (const auto &parameter : function.params) {
		auto paramType = resolveType(parameter.type);
		if (!paramType.has_value()) {
			messageBag.bug(parameter.getToken(), "TYPE-CHECKER",
			               std::format("could not inspect type for {}",
			                           parameter.type.name.lexeme));
			continue;
		}
		auto parameterType = paramType.value();
		if (parameterType.calculatedSize == 0) {
			messageBag.error(
			    parameter.type.getToken(), "TYPE-CHECKER",
			    std::format(
			        "cannot pass parameter type with unknown size for '{}'",
			        parameterType.name));
			return;
		}

		parameters.push_back({
		    .name = parameter.name.lexeme,
		    .parameterType = parameterType,
		});
	}

	auto functionReturnType = resolveType(function.returnType);
	if (!functionReturnType.has_value()) {
		return;
	}
	auto returnType = functionReturnType.value();
	if (!returnType.isScalar() && returnType.calculatedSize == 0) {
		messageBag.error(
		    function.returnType.getToken(), "TYPE-CHECKER",
		    std::format("cannot return a type with unknown size for '{}'",
		                returnType.name));
		return;
	}

	auto declaration = lang::FunctionDeclaration{
	    .name = std::string(function.name.getLexeme()),
	    .mangledName = mangledFunctionName,
	    .parameters = parameters,
	    .publicVisibility = function.publicVisibility,
	    .returnType = returnType,
	};
	auto definition = lang::FunctionDefinition{
	    .name = std::string(function.name.getLexeme()),
	    .mangledName = mangledFunctionName,
	    .function = function,
	    .returnType = returnType,
	};

	currentSourceUnit.functionDeclarations.push_back(declaration);
	if (function.body.has_value()) {
		currentSourceUnit.functionDefinitions.push_back(definition);
		resolveTypes(function.body.value());
	}
	// TODO: return a function pointer type with an specific signature
	std::stringstream signature;
	for (size_t i = 0; i < parameters.size(); ++i) {
		signature << parameters[i].parameterType.name;
		if (i + 1 < parameters.size()) {
			signature << ",";
		}
	}
	auto functionType = lang::Type::defineFunctionType(signature.str());
	typeStack.push_back(functionType);
}
void TypeChecker::visitIfStatement(const ast::If &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitJumpStatement(const ast::Jump &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitVarStatement(const ast::Var &value) {
	auto type = lang::Type{};

	if (value.type.token.type != Token::TokenType::TOKEN_UNINITIALIZED) {
		// initialization code
	}

	auto initializationType = lang::Type{};
	if (value.initializer.has_value()) {
		auto &initializer = *value.initializer.value().get();
		auto initType = resolveType(initializer);
		if (initType.has_value()) {
			initializationType = initType.value();
		}
	}

	if (!(type.isInitialized() || initializationType.isInitialized())) {
		messageBag.error(value.getToken(), "TYPE-ERROR",
		                 "variable does not have a type assigned nor an valid "
		                 "initialization");
	}
}
void TypeChecker::visitWhileStatement(const ast::While &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
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
void TypeChecker::visitVariableExpression(const ast::Variable &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitAssignExpression(const ast::Assign &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitBinaryExpression(const ast::Binary &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitCallExpression(const ast::Call &value) {

	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
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
			messageBag.error(
			    intrinsicCall.callee->name, "TYPE-CHECKER",
			    std::format("'{}' is not implemented yet for type checker",
			                intrinsicCall.callee->name.lexeme));
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
void TypeChecker::visitGroupingExpression(const ast::Grouping &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitLiteralExpression(const ast::Literal &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
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
void TypeChecker::visitUnaryExpression(const ast::Unary &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitArrayAccessExpression(const ast::ArrayAccess &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
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
		pointerType.isConst = typeExpr.isConst;
		typeStack.push_back(pointerType);
	} else {
		auto result = findTypeInfo(typeExpr.name.lexeme);
		if (result.has_value()) {
			lang::Type obtainedType = result.value();
			obtainedType.isConst = typeExpr.isConst;
			typeStack.push_back(obtainedType);
		} else {
			messageBag.error(
			    typeExpr.getToken(), "TYPE-CHECKER",
			    std::format("type not found for {}", typeExpr.name.lexeme));
		}
	}
}
void TypeChecker::visitCastExpression(const ast::Cast &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}
void TypeChecker::visitParameterExpression(const ast::Parameter &value) {
	messageBag.bug(value.getToken(), "TYPE-CHECKER",
	               std::format("visit method not implemented for {}",
	                           value.variantName()));
}

std::optional<lang::Type>
TypeChecker::resolveType(const ast::Statement &statement) {
	auto types = resolveTypes(statement);

	if (types.size() > 1) {
		messageBag.bug(
		    statement.getToken(), "TYPE-CHECKER",
		    std::format("'{}' yield multiple values", statement.variantName()));
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
	if (returnTypes.size() < 1) {
		messageBag.bug(statement.getToken(), "TYPE-CHECKER",
		               std::format("'{}' did not yield any return type",
		                           statement.variantName()));
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
std::optional<lang::Type>
TypeChecker::getTypeExpression(const ast::Expression *expression) {
	if (auto var = dynamic_cast<const ast::Variable *>(expression)) {
		return findTypeInfo(var->name.lexeme);
	}
	return {};
}

lang::Type TypeChecker::makePointerType(const lang::Type &innerType) {
	return lang::Type(true,
	                  // a pointer is not a scalar as it is an address memory
	                  // that references an object
	                  false,
	                  // technically platform dependent on pointer definition
	                  true,
	                  // define the name as pointer
	                  std::format("pointer", innerType.name),
	                  // for now lets just copy the type name
	                  std::format("pointer", innerType.name),
	                  // we need to get this from the platform in the future
	                  // for now assuming 64bits/8bytes
	                  8,
	                  // if the pointer type is const or not is decided later
	                  false,
	                  // we are a pointer type
	                  true,
	                  // a pointer is not a signed type
	                  false,
	                  // our inner pointer type
	                  {innerType});
}
} // namespace ray::compiler::analyzer
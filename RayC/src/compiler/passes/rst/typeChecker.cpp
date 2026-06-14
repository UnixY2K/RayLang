#include "ray/compiler/lang/type.hpp"
#include <format>

#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/passes/rst/typeChecker.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>

namespace ray::compiler::passes::rst {

void TypeChecker::resolve(syntax::rst::Block &rootBlock) {
	rootBlock.visit(*this);
}

bool TypeChecker::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TypeChecker::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TypeChecker::getWarnings() const {
	return messageBag.getWarnings();
}

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
    const syntax::rst::TerminalExpression &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitExpressionStatementStatement(
    const syntax::rst::ExpressionStatement &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
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
		const auto functionDeclaration = declarationResult.value();
		if (!currentSourceUnit.declareFunction(functionDeclaration,
		                                       currentScope)) {
			messageBag.error(functionRST.getToken(),
			                 "could not declare function");
		}

		auto definition = lang::FunctionDefinition{
		    .declaration = functionDeclaration,
		    .function = functionRST,
		};
		std::vector<util::copy_ptr<lang::Type>> paramTypes;
		for (const auto &param : functionDeclaration.signature.parameters) {
			paramTypes.push_back(
			    util::copy_ptr<lang::Type>(param.parameterType));
		}

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
				        "inner body return type does not match with function return: '{}' vs '{}'",
				        type.name,
				        functionDeclaration.signature.returnType.name));
			}

			currentScope = parentScope;
		}

		auto functionType = currentDataModel.get().defineFunctionType(
		    functionDeclaration.signature.returnType, paramTypes);
		typeStack.push_back(functionType);
	}
}
void TypeChecker::visitIfStatement(const syntax::rst::If &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitJumpStatement(const syntax::rst::Jump &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitVarDeclStatement(const syntax::rst::VarDecl &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitMemberStatement(const syntax::rst::Member &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitWhileStatement(const syntax::rst::While &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitStructStatement(const syntax::rst::Struct &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitPlaceholderStatement(
    const syntax::rst::Placeholder &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}

// expression visitor
void TypeChecker::visitVariableExpression(const syntax::rst::Variable &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitIntrinsicExpression(
    const syntax::rst::Intrinsic &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitAssignExpression(const syntax::rst::Assign &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitBinaryExpression(const syntax::rst::Binary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitCallExpression(const syntax::rst::Call &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
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
void TypeChecker::visitGroupingExpression(const syntax::rst::Grouping &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitLiteralExpression(const syntax::rst::Literal &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitLogicalExpression(const syntax::rst::Logical &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitSetExpression(const syntax::rst::Set &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitUnaryExpression(const syntax::rst::Unary &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitArrayAccessExpression(
    const syntax::rst::ArrayAccess &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitArrayTypeExpression(
    const syntax::rst::ArrayType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitTupleTypeExpression(
    const syntax::rst::TupleType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void TypeChecker::visitPointerTypeExpression(
    const syntax::rst::PointerType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
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
void TypeChecker::visitCastExpression(const syntax::rst::Cast &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
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
TypeChecker::resolveFunctionDeclaration(
    const syntax::rst::Function &functionRST) {
	std::string currentModule = "root";

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
	        currentModule, functionRST, linkageDirective);

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
	        }).value_or(lang::Type::defineUnknownType());

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
		// pointer type was already evaluated and thus should be already a known
		// type
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

	auto declaration = lang::FunctionDeclaration{
	    .name = std::string(functionRST.name.getLexeme()),
	    .mangledName = mangledFunctionName,
	    .publicVisibility = functionRST.publicVisibility,
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

} // namespace ray::compiler::passes::rst
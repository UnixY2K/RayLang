#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/intrinsic.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/generators/c/c_transpiler.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::generator::c {

std::string CTranspilerGenerator::currentIdent() const {
	return std::string(ident, '\t');
}

CTranspilerGenerator::CTranspilerGenerator(
    std::string filePath, const lang::SourceUnit &sourceUnit,
    const environment::DataModel &dataModel)
    : messageBag("C-BACKEND", filePath), currentSourceUnit(sourceUnit),
      currentScope(sourceUnit.rootScope), dataModel(dataModel) {}

void CTranspilerGenerator::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement) {
	output.clear();

	output << "#include <ray/ray_definitions.h>\n";
	output << "#ifdef __cplusplus\n";
	output << "RAY_C_LINKAGE {\n";
	output << "#endif\n";

	std::string currentModule;
	output << "#pragma region struct_declarations\n";
	for (auto const &[structId, structDeclaration] :
	     currentSourceUnit.get().getStructs()) {
		output << std::format("{}typedef struct {}", currentIdent(),
		                      structDeclaration.mangledName);
		output << std::format(" {};\n", structDeclaration.mangledName);
	}
	output << "#pragma endregion struct_declarations\n";

	output << "#pragma region struct_definitions\n";
	// struct cyclic dependency check is done at type check step
	std::unordered_set<size_t> visitedStructs;
	visitedStructs.reserve(currentSourceUnit.get().getStructs().size());
	for (auto const &[structId, structDeclaration] :
	     currentSourceUnit.get().getStructs()) {
		defineStruct(visitedStructs, structDeclaration);
	}
	output << "#pragma endregion struct_definitions\n";

	output << "#pragma region function_declarations\n";
	for (const auto &[functionId, functionDeclaration] :
	     currentSourceUnit.get().getFunctions()) {
		// main should be extern c++
		if (functionDeclaration.mangledName == "main") {
			output << "RAY_DEFAULT_LINKAGE ";
		}
		if (!functionDeclaration.publicVisibility) {
			output << "RAYLANG_MACRO_LINK_LOCAL ";
			output << "static ";
		}
		visitType(functionDeclaration.signature.returnType);

		output << std::format(" {}(", functionDeclaration.mangledName);
		for (size_t index = 0;
		     index < functionDeclaration.signature.parameters.size(); ++index) {
			const auto &parameter =
			    functionDeclaration.signature.parameters[index];
			visitType(parameter.parameterType);
			output << std::format(" {}", parameter.name);
			if (index < functionDeclaration.signature.parameters.size() - 1) {
				output << ", ";
			}
		}
		output << ");\n";
	}
	output << "#pragma endregion function_declarations\n";
	// ident++;
	for (const auto &stmt : statement) {
		stmt->visit(*this);
	}
	// ident--;
	output << "#ifdef __cplusplus\n";
	output << "}\n";
	output << "#endif\n";

	if (!this->directivesStack.empty()) {
		for (auto &directive : directivesStack) {
			messageBag.warning(directive->getToken(),
			                   std::format("unused compiler directive {}",
			                               directive->directiveName()));
		}
	}
}

bool CTranspilerGenerator::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> CTranspilerGenerator::getErrors() const {
	return messageBag.getErrors();
}

std::string CTranspilerGenerator::getOutput() const { return output.str(); }

// Statement
void CTranspilerGenerator::visitBlockStatement(const ast::Block &block) {
	if (block.statements.size() > 0) {
		for (auto &statement : block.statements) {
			statement->visit(*this);
		}
	}
}
void CTranspilerGenerator::visitTerminalExprStatement(
    const ast::TerminalExpr &terminalExpr) {
	if (terminalExpr.expression.has_value()) {
		terminalExpr.expression->get()->visit(*this);
		output << std::format("{}return;\n", currentIdent());
	}
}
void CTranspilerGenerator::visitExpressionStmtStatement(
    const ast::ExpressionStmt &expression) {
	output << currentIdent();
	expression.expression->visit(*this);
	output << std::format(";\n", currentIdent());
}
void CTranspilerGenerator::visitFunctionStatement(
    const ast::Function &function) {
	std::string identTabs = currentIdent();
	std::string currentModule;

	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > top; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(),
			    std::format("unmatched compiler directive '{}' for function.",
			                directive->directiveName()));
		}
		directivesStack.pop_back();
	}
	std::string functionName =
	    nameMangler.mangleFunction(currentModule, function, linkageDirective);

	// ignore any function declaration
	if (function.body.has_value()) {

		output << identTabs;
		// main has special rules to linking that we must follow
		if (functionName == "main") {
			output << "RAY_DEFAULT_LINKAGE ";
		} else {
			if (function.publicVisibility) {
				output << "RAYLANG_MACRO_LINK_EXPORT ";
			} else {
				output << "RAYLANG_MACRO_LINK_LOCAL ";
				output << "static ";
			}
		}

		function.returnType->visit(*this);

		output << std::format("{}(", functionName);
		for (size_t index = 0; index < function.params.size(); ++index) {
			const auto &parameter = function.params[index];
			parameter.visit(*this);
			if (index < function.params.size() - 1) {
				output << ", ";
			}
		}
		output << ")";
		output << " {";
		if (function.body->statements.size() > 0) {
			auto statement = dynamic_cast<ast::TerminalExpr *>(
			    function.body->statements[0].get());
			if (!statement || statement->expression.has_value()) {
				output << "\n";
				ident++;
				function.body->visit(*this);
				ident--;
			}
		}
		output << std::format("{}}}\n", identTabs);
	}
}
void CTranspilerGenerator::visitIfStatement(const ast::If &ifStatement) {
	output << std::format("{}if (", currentIdent());
	ifStatement.condition->visit(*this);
	output << ") {\n";
	ident++;
	ifStatement.thenBranch->visit(*this);
	ident--;
	output << std::format("{}}}\n", currentIdent());
	if (ifStatement.elseBranch.has_value()) {
		output << std::format("{}else{{\n", currentIdent());
		ident++;
		ifStatement.elseBranch->get()->visit(*this);
		ident--;
		output << std::format("{}}}\n", currentIdent());
	}
}
void CTranspilerGenerator::visitJumpStatement(const ast::Jump &jump) {
	std::string identTab = currentIdent();
	switch (jump.keyword.type) {
	case Token::TokenType::TOKEN_BREAK:
		output << std::format("{}br 0\n", identTab);
		break;
	case Token::TokenType::TOKEN_CONTINUE:
		output << std::format("{}br 1\n", identTab);
		break;
	case Token::TokenType::TOKEN_RETURN:
		output << std::format("{}return", identTab);
		if (jump.value.has_value()) {
			output << " ";
			auto currentIdent = ident;
			ident = 0;
			jump.value.value()->visit(*this);
			ident = currentIdent;
		}
		output << ";\n";
		break;
	default:
		messageBag.error(jump.getToken(),
		                 std::format("'{}' is not a supported jump type",
		                             jump.keyword.getLexeme()));
		break;
	}
}
void CTranspilerGenerator::visitVarDeclStatement(const ast::VarDecl &var) {
	output << currentIdent();

	var.type->visit(*this);
	output << std::format("{}", var.name.lexeme);

	if (var.initializer.has_value()) {
		output << " = ";
		auto initializer = var.initializer->get();
		auto currentIdent = ident;
		ident = 0;
		initializer->visit(*this);
		ident = currentIdent;
	}
	output << ";\n";
}
void CTranspilerGenerator::visitMemberStatement(const ast::Member &var) {
	output << currentIdent();

	var.type->visit(*this);
	output << std::format("{}", var.name.lexeme);

	if (var.initializer.has_value()) {
		output << " = ";
		auto initializer = var.initializer->get();
		auto currentIdent = ident;
		ident = 0;
		initializer->visit(*this);
		ident = currentIdent;
	}
	output << ";\n";
}
void CTranspilerGenerator::visitWhileStatement(const ast::While &value) {
	auto identTab = currentIdent();
	output << std::format("{}while (", identTab);
	auto currentIdent = ident;
	ident = 0;
	value.condition->visit(*this);
	ident = currentIdent;
	output << ") {\n";
	ident++;
	value.body->visit(*this);
	ident--;
	output << std::format("{}}}\n", identTab);
}
void CTranspilerGenerator::visitStructStatement(const ast::Struct &value) {
	// TODO: remove this once the full logic of struct using type data is
	// implemented
	return;

	std::string currentModule;

	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > top; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			messageBag.warning(
			    directive->getToken(),
			    std::format("unmatched compiler directive '{}' for function.",
			                directive->directiveName()));
		}
		directivesStack.pop_back();
	}
	const std::string mangledStructName =
	    nameMangler.mangleStruct(currentModule, value, linkageDirective);

	// we just ignore any struct declaration
	// as they were declared before
	if (!value.declaration) {

		output << std::format("{}typedef struct {}", currentIdent(),
		                      mangledStructName);
		output << " {\n";
		ident++;
		for (auto &member : value.members) {
			member.visit(*this);
		}
		if (value.members.empty()) {
			// make a char field so on both C and C++ holds 1 byte
			// still this field should never be used
			// and its fields should not be accesible
			// TODO: make a method in the mangler to create reserved
			// variable/member names
			output << std::format("{}const u8 _rayREmptyStruct__;\n",
			                      currentIdent());
		}
		ident--;
		output << std::format("{}}}", currentIdent());
		output << std::format(" {};\n", mangledStructName);
	}
}
void CTranspilerGenerator::visitCompDirectiveStatement(
    const ast::CompDirective &compDirective) {
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
		    compDirective.getToken());
		if (compDirective.child) {
			auto childValue = compDirective.child.get();
			if (dynamic_cast<ast::Function *>(childValue) ||
			    dynamic_cast<ast::Struct *>(childValue)) {
				size_t startDirectives = directivesStack.size();
				size_t originalTop = top + 1;
				top = startDirectives;
				directivesStack.push_back(
				    std::make_unique<directive::LinkageDirective>(directive));
				compDirective.child->visit(*this);
				if (directivesStack.size() != startDirectives) {
					messageBag.bug(childValue->getToken(),
					               "unprocessed compiler directives");
				}
				top = originalTop;
			} else {
				messageBag.error(
				    childValue->getToken(),
				    std::format(
				        "{} child expression must be a function or a struct.",
				        directive.directiveName()));
			}
		} else {
			messageBag.error(compDirective.getToken(),
			                 std::format("{} must have a child expression.",
			                             directive.directiveName()));
		}
	} else {
		messageBag.error(
		    compDirective.getToken(),
		    std::format("Unknown compiler directive '{}'.", directiveName));
	}
}
// Expression
void CTranspilerGenerator::visitVariableExpression(
    const ast::Variable &variable) {
	output << std::format("{}", variable.name.lexeme);
}
void CTranspilerGenerator::visitIntrinsicExpression(
    const ast::Intrinsic &intrinsic) {
	messageBag.error(intrinsic.name,
	                 "visitIntrinsicExpression not implemented");
}
void CTranspilerGenerator::visitAssignExpression(const ast::Assign &value) {
	value.lhs->visit(*this);
	output << std::format(" {} ", value.assignmentOp.getGlyph());
	value.rhs->visit(*this);
}
void CTranspilerGenerator::visitBinaryExpression(
    const ast::Binary &binaryExpression) {
	std::string identTab = currentIdent();
	binaryExpression.left->visit(*this);

	auto op = binaryExpression.op;
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
	case Token::TokenType::TOKEN_GREAT_GREAT:
	case Token::TokenType::TOKEN_EQUAL_EQUAL:
	case Token::TokenType::TOKEN_BANG_EQUAL:
	case Token::TokenType::TOKEN_LESS:
	case Token::TokenType::TOKEN_GREAT:
	case Token::TokenType::TOKEN_LESS_EQUAL:
	case Token::TokenType::TOKEN_GREAT_EQUAL:
		output << std::format(" {} ", op.getGlyph());
		break;
	default:
		messageBag.error(binaryExpression.op,
		                 std::format("'{}' is not a supported binary operation",
		                             op.getLexeme()));
	}

	binaryExpression.right->visit(*this);
}
void CTranspilerGenerator::visitCallExpression(const ast::Call &callable) {
	// check if the callable contains a function
	if (ast::Variable *var =
	        dynamic_cast<ast::Variable *>(callable.callee.get())) {
		std::string callableName =
		    findCallableName(callable, var->name.getLexeme());
		if (callableName.empty()) {
			messageBag.error(var->name, std::format("undefined symbol '{}'",
			                                        var->name.lexeme));
			callableName = var->name.lexeme;
		}
		output << std::format("{}(", callableName);

		for (size_t index = 0; index < callable.arguments.size(); ++index) {
			auto const &argument = callable.arguments[index];
			auto currentIdent = ident;
			ident = 0;
			argument->visit(*this);
			ident = currentIdent;
			if (index < callable.arguments.size() - 1) {
				output << ", ";
			}
		}
		output << ")";
	} else {
		messageBag.error(callable.callee->getToken(),
		                 std::format("'{}' is not a supported callable type",
		                             callable.callee.get()->variantName()));
	}
}
void CTranspilerGenerator::visitIntrinsicCallExpression(
    const ast::IntrinsicCall &value) {

	switch (value.callee->intrinsic) {
	case ray::compiler::ast::IntrinsicType::INTR_SIZEOF: {
		if (value.arguments.size() != 1) {
			messageBag.error(value.callee->name,
			                 std::format("@sizeOf intrinsic expects 1 "
			                             "argument but {} got provided",
			                             value.arguments.size()));
		} else {
			auto param = value.arguments[0].get();
			if (auto type = getTypeExpression(param)) {
				output << std::format("((ssize){})", type->calculatedSize);
			} else {
				messageBag.error(value.callee->name,
				                 std::format("'{}' is not a Type expression",
				                             param->variantName()));
			}
		}
		break;
	}
	case ray::compiler::ast::IntrinsicType::INTR_IMPORT: {
		messageBag.error(
		    value.callee->name,
		    std::format("'{}' is not implemented yet for C backend",
		                value.callee->name.lexeme));
		break;
	}
	case ray::compiler::ast::IntrinsicType::INTR_UNKNOWN:
		messageBag.error(value.callee->name,
		                 std::format("'{}' is not a valid intrinsic",
		                             value.callee->name.lexeme));
		break;
	}
}
void CTranspilerGenerator::visitGetExpression(const ast::Get &value) {
	value.object->visit(*this);
	output << std::format(".{}", value.name.lexeme);
}
void CTranspilerGenerator::visitGroupingExpression(
    const ast::Grouping &grouping) {
	grouping.expression->visit(*this);
}
void CTranspilerGenerator::visitLiteralExpression(const ast::Literal &literal) {
	switch (literal.kind.type) {
	case Token::TokenType::TOKEN_TRUE:
	case Token::TokenType::TOKEN_FALSE:
		output << std::format(
		    "{}", literal.kind.type == Token::TokenType::TOKEN_TRUE ? "true"
		                                                            : "false");
		break;
	case Token::TokenType::TOKEN_STRING: {
		output << "(const u8[]){";
		for (const char c : literal.value) {
			output << std::format("0x{:02X}, ", c);
		}
		output << "0x00}";
		// comment string literal
		output << "/*\"";
		for (const char c : literal.value) {
			switch (c) {
			case '\a':
				output << "\\a";
				break;
			case '\b':
				output << "\\b";
				break;
			case '\e':
				output << "\\e";
				break;
			case '\f':
				output << "\\f";
				break;
			case '\n':
				output << "\\n";
				break;
			case '\r':
				output << "\\r";
				break;
			case '\v':
				output << "\\v";
				break;
			case '\'':
				output << "'";
				break;
			case '"':
				output << '"';
				break;
			case '?':
				output << '?';
				break;
			default:
				output << c;
			}
		}
		output << "\"*/";
		break;
	}
	case Token::TokenType::TOKEN_NUMBER: {
		std::string_view value = literal.value;
		bool commaFound = false;
		size_t end_pos = 0;
		for (; end_pos < value.size(); end_pos++) {
			const char digit = value[end_pos];
			int number = digit - '0';
			if (number >= 0 && number <= 9) {
				continue;
			}
			if (digit == '.') {
				if (commaFound) {
					break;
				}
				commaFound = true;
				continue;
			}
			break;
		}

		// read until no more digits found
		value = value.substr(0, end_pos);
		output << std::format("{}", value);
		break;
	}
	case Token::TokenType::TOKEN_CHAR: {
		output << std::format("(const u8){{0x{:02X}}}", literal.value[0]);
		break;
	}
	default:
		messageBag.error(
		    literal.token,
		    std::format("'{}' ({}) is not a supported literal type",
		                literal.kind.getLexeme(), literal.kind.getGlyph()));
		break;
	}
}
void CTranspilerGenerator::visitLogicalExpression(
    const ast::Logical &logicalExpr) {
	output << "(bool)(";
	logicalExpr.left->visit(*this);
	output << std::format(" {} ", logicalExpr.op.getGlyph());
	logicalExpr.right->visit(*this);
	output << ")";
}
void CTranspilerGenerator::visitSetExpression(const ast::Set &value) {
	value.object->visit(*this);
	output << std::format(".{} {} ", value.name.lexeme,
	                      value.assignmentOp.getGlyph());
	value.value->visit(*this);
}
void CTranspilerGenerator::visitUnaryExpression(const ast::Unary &unary) {
	if (!unary.isPrefix) {
		unary.expr->visit(*this);
	}
	switch (unary.op.type) {
	case Token::TokenType::TOKEN_BANG:
	case Token::TokenType::TOKEN_MINUS:
	case Token::TokenType::TOKEN_MINUS_MINUS:
	case Token::TokenType::TOKEN_PLUS_PLUS:
		output << std::format("{}", unary.op.getLexeme());
		break;
	default:
		messageBag.error(unary.op,
		                 std::format("'{}' is not a supported unary operation",
		                             unary.op.getLexeme()));
	}
	if (unary.isPrefix) {
		unary.expr->visit(*this);
	}
}
void CTranspilerGenerator::visitArrayAccessExpression(
    const ast::ArrayAccess &value) {
	value.array->visit(*this);
	output << "[";
	value.index->visit(*this);
	output << "]";
}
void CTranspilerGenerator::visitArrayTypeExpression(
    const ast::ArrayType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void CTranspilerGenerator::visitTupleTypeExpression(
    const ast::TupleType &value) {
	messageBag.error(value.getToken(),
	                 std::format("{} not implemented", __PRETTY_FUNCTION__));
}
void CTranspilerGenerator::visitPointerTypeExpression(
    const ast::PointerType &pointerTypeAst) {

	pointerTypeAst.subtype->visit(*this);

	output << "*";
	if (!pointerTypeAst.isMutable) {
		output << " const ";
	}
}
void CTranspilerGenerator::visitNamedTypeExpression(
    const ast::NamedType &type) {
	if (!type.isMutable && type.name.lexeme != "void"
	    // && !type.isPointer // TODO: make this section use the type checker
	    // instead
	) {
		output << "const ";
	}
	if (type.name.lexeme == "void") {
		output << "void ";
	} else {
		// TODO: replace this to a typeID System
		std::string typeName = findStructName(type.name.lexeme);
		if (typeName.empty()) {
			auto typeInfo = findTypeInfo(type.name.lexeme);
			if (typeInfo.has_value()) {
				typeName = typeInfo->name;
			} else {
				messageBag.warning(
				    type.getToken(),
				    std::format("could not find mangled name for '{}'",
				                type.name.lexeme));
			}
		}
		typeName = typeName.empty() ? type.name.lexeme : typeName;
		output << std::format("{} ", typeName);
	}
}
void CTranspilerGenerator::visitCastExpression(const ast::Cast &value) {
	output << "(";
	value.type->visit(*this);
	output << ")(";
	value.expression->visit(*this);
	output << ")";
}
void CTranspilerGenerator::visitParameterExpression(
    const ast::Parameter &param) {
	param.type->visit(*this);
	output << std::format("{}", param.name.lexeme);
}

void CTranspilerGenerator::visitType(const lang::Type &type) {
	// all types except pointer have const before its type
	if (!type.isMutable && type.getKind() != lang::TypeKind::pointer) {
		output << "const ";
	}
	switch (type.getKind()) {
	case lang::TypeKind::pointer: {
		visitType(*type.subtype.value());
		output << "*";
		if (!type.isMutable) {
			output << "const";
		}
		break;
	}
	case lang::TypeKind::scalar:
		output << type.name;
		break;
	case lang::TypeKind::aggregate:
		// see if the type is a struct and get its mangled name
		if (currentSourceUnit.get().getStructs().contains(type.typeId)) {
			const lang::Struct &structObj =
			    currentSourceUnit.get().getStructs().at(type.typeId);
			output << structObj.mangledName;
		} else if (currentSourceUnit.get().getFunctions().contains(
		               type.typeId)) {
			messageBag.bug(
			    types::makeUnitTypeToken(0, 0),
			    std::format("function name mangling not yet supported"));

		} else {
			messageBag.bug(
			    types::makeUnitTypeToken(0, 0),
			    std::format("could not identify aggregate name information"));
		}
		break;
	case lang::TypeKind::abstract:
		messageBag.bug(types::makeUnitTypeToken(0, 0),
		               std::format("abstract tokens cannot be transpiled"));
		break;
	}
}

std::string
CTranspilerGenerator::findCallableName(const ast::Call &callable,
                                       const std::string_view name) const {
	// TODO: replace this to a resolved lookup done by the type checker
	// once the type checker performs the binding

	std::string key(name);
	const std::string functionName =
	    currentScope.get()
	        .findFunctionDeclaration(name)
	        .transform(
	            [&callable](
	                const std::vector<
	                    util::soft_reference<lang::FunctionDeclaration>>
	                    &functionDeclarations) -> std::optional<std::string> {
		            for (const auto &function : functionDeclarations) {
			            const auto &functionObject = function.getObject();
			            if (functionObject->get().signature.parameters.size() ==
			                callable.arguments.size()) {
				            return functionObject->get().mangledName;
			            }
		            }
		            return std::nullopt;
	            })
	        ->value_or(std::string());

	return functionName;
}
std::string
CTranspilerGenerator::findStructName(const std::string_view name) const {
	auto queriedStruct =
	    currentSourceUnit.get().findStruct(name, currentScope.get());
	if (queriedStruct.has_value()) {
		return queriedStruct
		    .transform(
		        [](const auto structValue) -> std::optional<std::string> {
			        return structValue.get().mangledName;
		        })
		    ->value_or(std::string());
	}

	return std::string();
}

std::optional<lang::Type>
CTranspilerGenerator::findScalarTypeInfo(const std::string_view lexeme) {
	return dataModel.get().findScalarType(lexeme);
}
std::optional<lang::Type>
CTranspilerGenerator::findTypeInfo(const std::string_view lexeme) {
	auto scalarType = findScalarTypeInfo(lexeme);
	if (scalarType) {
		return scalarType;
	}
	return {};
}
std::optional<lang::Type>
CTranspilerGenerator::getTypeExpression(const ast::Expression *expression) {
	if (auto var = dynamic_cast<const ast::Variable *>(expression)) {
		return findTypeInfo(var->name.lexeme);
	}
	return {};
}

void CTranspilerGenerator::defineStruct(
    std::unordered_set<size_t> &visitedStructs, const lang::Struct &structObj) {
	if (visitedStructs.contains(structObj.structID)) {
		return;
	}
	visitedStructs.insert(structObj.structID);

	if (structObj.members.size() == 0) {
		return;
	}

	for (auto const &structMember : structObj.members) {
		auto typeId = structMember.type.typeId;
		if (currentSourceUnit.get().getStructs().contains(typeId)) {
			const lang::Struct structDeclaration =
			    currentSourceUnit.get().getStructs().at(typeId);
			defineStruct(visitedStructs, structDeclaration);
		}
	}

	// TODO: read compiler directives from type checker

	output << std::format("typedef struct {} {{\n", structObj.mangledName);
	for (auto const &structMember : structObj.members) {
		output << std::format("\t");
		visitType(structMember.type);
		output << std::format(" {}; {}\n", structMember.name,
		                      structMember.publicVisibility ? "//#private"
		                                                    : "//#public");
	}

	output << std::format("}} {};\n", structObj.mangledName);
}

} // namespace ray::compiler::generator::c

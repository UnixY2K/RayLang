#include <cstddef>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/intrinsic.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/directives/compilerDirective.hpp>
#include <ray/compiler/directives/linkageDirective.hpp>
#include <ray/compiler/generators/c/c_transpiler.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/message_bag.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/topLevelResolver.hpp>

namespace ray::compiler::generator::c {

using namespace terminal::literals;

std::string CTranspilerGenerator::currentIdent() const {
	return std::string(ident, '\t');
}

void CTranspilerGenerator::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement,
    const lang::SourceUnit &sourceUnit) {
	output.clear();
	symbolTable.clear();

	output << "#include <ray/ray_definitions.h>\n";
	output << "#ifdef __cplusplus\n";
	output << "extern \"C\" {\n";
	output << "#endif\n";

	std::string currentModule;
	// TODO: we currently iterate 2 times one for declaration
	// while the other for definition
	for (const auto &structDeclaration : sourceUnit.structDeclarations) {
		output << std::format("{}typedef struct {}", currentIdent(),
		                      structDeclaration.mangledName);
		output << std::format(" {};\n", structDeclaration.mangledName);
	}
	for (const auto &structDefinition : sourceUnit.structDefinitions) {
		auto &structObj = structDefinition.structObj.get();
		output << std::format("{}typedef struct {}", currentIdent(),
		                      structDefinition.mangledName);
		output << " {\n";
		ident++;
		for (auto &member : structObj.members) {
			member.visit(*this);
		}
		ident--;
		output << std::format("{}}}", currentIdent());
		output << std::format(" {};\n", structDefinition.mangledName);
	}
	for (const lang::FunctionDeclaration &functionDeclaration :
	     sourceUnit.functionDeclarations) {
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
			std::cerr << std::format("{}: unused compiler directive {}\n",
			                         "WARNING"_yellow,
			                         directive->directiveName());
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
			std::cout << std::format(
			    "{}: unmatched compiler directive '{}' for function.\n",
			    "WARNING"_yellow, directive->directiveName());
		}
		directivesStack.pop_back();
	}
	std::string functionName =
	    nameMangler.mangleFunction(currentModule, function, linkageDirective);

	// ignore any function declaration
	if (function.body.has_value()) {

		output << identTabs;
		if (!function.publicVisibility) {
			output << "RAYLANG_MACRO_LINK_LOCAL ";
			output << "static ";
		} else if (function.body.has_value()) {
			if (function.body.has_value()) {
				output << "RAYLANG_MACRO_LINK_EXPORT ";
			} else {
				output << "RAYLANG_MACRO_LINK_IMPORT ";
			}
		}
		function.returnType.visit(*this);

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
		if (function.body->statements.size() > 1) {
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
		messageBag.error(jump.getToken(), "C-BACKEND",
		                 std::format("'{}' is not a supported jump type",
		                             jump.keyword.getLexeme()));
		break;
	}
}
void CTranspilerGenerator::visitVarStatement(const ast::Var &var) {
	output << currentIdent();

	var.type.visit(*this);
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
	std::string currentModule;

	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > top; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			std::cout << std::format(
			    "{}: unmatched compiler directive '{}' for function.\n",
			    "WARNING"_yellow, directive->directiveName());
		}
		directivesStack.pop_back();
	}

	// we just ignore any struct declaration
	// as they were declared before
	if (!value.declaration) {
		output << std::format("{}typedef struct {}", currentIdent(),
		                      value.name.lexeme);
		output << " {\n";
		ident++;
		for (auto &member : value.members) {
			member.visit(*this);
		}
		ident--;
		output << std::format("{}}}", currentIdent());
		output << std::format(" {};\n", value.name.lexeme);
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
					messageBag.error(childValue->getToken(), "BUG",
					                 "unprocessed compiler directives");
				}
				top = originalTop;
			} else {
				messageBag.error(
				    childValue->getToken(), "C-BACKEND",
				    std::format(
				        "{} child expression must be a function or a struct.",
				        directive.directiveName()));
			}
		} else {
			messageBag.error(compDirective.getToken(), "C-BACKEND",
			                 std::format("{} must have a child expression.",
			                             directive.directiveName()));
		}
	} else {
		messageBag.error(
		    compDirective.getToken(), "C-BACKEND",
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
	messageBag.error(intrinsic.name, "C-BACKEND",
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
		messageBag.error(binaryExpression.op, "C-BACKEND",
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
			messageBag.error(
			    var->name, "C-BACKEND",
			    std::format("undefined symbol '{}'", var->name.lexeme));
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
		messageBag.error(callable.callee->getToken(), "C-BACKEND",
		                 std::format("'{}' is not a supported callable type",
		                             callable.callee.get()->variantName()));
	}
}
void CTranspilerGenerator::visitIntrinsicCallExpression(
    const ast::IntrinsicCall &value) {

	switch (value.callee->intrinsic) {
	case ray::compiler::ast::IntrinsicType::INTR_SIZEOF: {
		if (value.arguments.size() != 1) {
			messageBag.error(value.callee->name, "C-BACKEND",
			                 std::format("@sizeOf intrinsic expects 1 "
			                             "argument but {} got provided",
			                             value.arguments.size()));
		} else {
			auto param = value.arguments[0].get();
			if (auto type = getTypeExpression(param)) {
				if (type->isPlatformDependent()) {
					output << std::format("((ssize)sizeof({}))",
					                      type->mangledName);
				} else {
					output << std::format("((ssize){})", type->calculatedSize);
				}
			} else {
				messageBag.error(value.callee->name, "C-BACKEND",
				                 std::format("'{}' is not a Type expression",
				                             param->variantName()));
			}
		}
		break;
	}
	case ray::compiler::ast::IntrinsicType::INTR_IMPORT: {
		messageBag.error(
		    value.callee->name, "C-BACKEND",
		    std::format("'{}' is not implemented yet for C backend",
		                value.callee->name.lexeme));
		break;
	}
	case ray::compiler::ast::IntrinsicType::INTR_UNKNOWN:
		messageBag.error(value.callee->name, "C-BACKEND",
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
		std::string value = literal.value;
		output << std::format("{}", value);
		break;
	}
	case Token::TokenType::TOKEN_CHAR: {
		output << std::format("(const u8){{0x{:02X}}}", literal.value[0]);
		break;
	}
	default:
		messageBag.error(
		    literal.token, "C-BACKEND",
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
		messageBag.error(unary.op, "C-BACKEND",
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
void CTranspilerGenerator::visitTypeExpression(const ast::Type &type) {
	if (!type.isMutable && type.name.type != Token::TokenType::TOKEN_TYPE_UNIT &&
	    !type.isPointer) {
		output << "const ";
	}
	if (type.name.type == Token::TokenType::TOKEN_TYPE_UNIT) {
		output << "void ";
	} else if (type.name.lexeme.starts_with("[")) {
		if (type.subtype.has_value() && type.subtype.value()) {
			type.subtype.value()->visit(*this);
			output << "*";
		} else {
			messageBag.error(type.name, "BUG", "subtype null for array type");
			output << std::format(
			    " *",
			    type.name.lexeme.substr(1,
			                            type.name.lexeme.find_last_of("]") - 1),
			    type.name.lexeme);
		}
	} else {
		if (type.isPointer) {
			if (type.subtype.has_value() && type.subtype.value()) {
				type.subtype.value()->visit(*this);
			} else {
				messageBag.error(type.name, "BUG", "subtype null");
			}
			output << "*";
		} else {
			std::string typeName = findStructName(type.name.lexeme);
			typeName = typeName.empty() ? type.name.lexeme : typeName;
			output << std::format("{} ", typeName);
		}
	}
}
void CTranspilerGenerator::visitCastExpression(const ast::Cast &value) {
	output << "(";
	value.type.visit(*this);
	output << ")(";
	value.expression->visit(*this);
	output << ")";
}
void CTranspilerGenerator::visitParameterExpression(
    const ast::Parameter &param) {
	param.type.visit(*this);
	output << std::format("{}", param.name.lexeme);
}

void CTranspilerGenerator::visitType(const lang::Type &type) {
	// for unit type we just use void
	if (type.name == "()") {
		output << "void";
		return;
	}
	if (type.isPointer) {
		visitType(*type.subtype.value());
		output << "*";
	} else {
		output << type.mangledName;
	}
}

std::string
CTranspilerGenerator::findCallableName(const ast::Call &callable,
                                       const std::string_view name) const {
	// we should make a propper lookup and ranking
	// but for now the first matching function will be used
	for (const auto &symbol : symbolTable) {
		if (symbol.name == name) {
			switch (symbol.type) {
			case lang::Symbol::SymbolType::Function: {
				if (const auto *function = dynamic_cast<const ast::Function *>(
				        symbol.object.value_or(nullptr))) {
					if (function->params.size() == callable.arguments.size()) {
						// here we should add to a list of candidate functions
						// than later are checked for types
						// but now we just return the mangled function name
						return symbol.mangledName;
					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
	return "";
}
std::string
CTranspilerGenerator::findStructName(const std::string_view name) const {
	// we should make a propper lookup and ranking
	// but for now we will match the first struct found with same name and
	// namespace
	for (const auto &symbol : symbolTable) {
		if (symbol.name == name) {
			switch (symbol.type) {
			case lang::Symbol::SymbolType::Struct: {
				if (const auto *currentStruct =
				        dynamic_cast<const ast::Struct *>(
				            symbol.object.value_or(nullptr))) {
					if (currentStruct->name.lexeme == name) {
						// here we should add to a list of candidate functions
						// than later are checked for types
						// but now we just return the mangled function name
						return symbol.mangledName;
					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
	return "";
}

std::optional<lang::Type>
CTranspilerGenerator::findScalarTypeInfo(const std::string_view lexeme) {
	return lang::Type::findScalarType(lexeme);
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

} // namespace ray::compiler::generator::c

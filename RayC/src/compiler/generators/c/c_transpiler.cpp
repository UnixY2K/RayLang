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
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/topLevelResolver.hpp>
#include <ray/compiler/types/types.hpp>

namespace ray::compiler::generator::c {

using namespace terminal::literals;

std::string CTranspilerGenerator::currentIdent() const {
	return std::string(ident, '\t');
}

void CTranspilerGenerator::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement,
    analyzer::symbols::SymbolTable symbolTable) {
	output.clear();
	this->symbolTable = symbolTable;

	output << "// this file was generated by RayLang C transpiler\n";
	output << "#ifdef __cplusplus\n";
	output << "extern \"C\" {\n";
	output << "#endif\n";
	output << "#include <stddef.h>\n";
	output << "#include <stdint.h>\n";
	// some of the definitions are technically UB as they may not be exactly 32
	// or 64 bit,ex(float, double implementations but generally they use the
	// IEEE 754 format) assuming that the following definitions are true
	output << "#define u8 uint8_t\n";
	output << "#define s8 int8_t\n";
	output << "#define u16 uint16_t\n";
	output << "#define s16 int16_t\n";
	output << "#define u32 uint32_t\n";
	output << "#define s32 int32_t\n";
	output << "#define u64 uint64_t\n";
	output << "#define s64 int64_t\n";
	output << "#define f32 float\n";
	output << "#define f64 double\n";
	output << "#define usize uintmax_t\n";
	output << "#define ssize intmax_t\n";
	output << "#define c_char char\n";
	output << "#define c_int int\n";
	output << "#define c_size size_t\n";
	output << "#define c_voidptr void *\n";
	output
	    << "#if defined(_WIN32) || defined(__CYGWIN__) || defined(_MSC_VER)\n";
	output << "// Microsoft\n";
	output << "#define RAYLANG_MACRO_LINK_IMPORT __declspec(dllimport)\n";
	output << "#define RAYLANG_MACRO_LINK_EXPORT __declspec(dllexport)\n";
	output << "#define RAYLANG_MACRO_LINK_LOCAL\n";
	output << "#elif defined(__GNUC__) && __GNUC__ >= 4\n";
	output << "// GCC\n";
	output << "#define RAYLANG_MACRO_LINK_IMPORT __attribute__((visibility"
	          "(\"default\")))\n";
	output << "#define RAYLANG_MACRO_LINK_EXPORT __attribute__((visibility"
	          "(\"default\")))\n";
	output << "#define RAYLANG_MACRO_LINK_LOCAL __attribute__((visibility"
	          "(\"hidden\")))\n";
	output << "#else\n";
	output << "#pragma warning \"Unknown dynamic link import/export "
	          "semantics.\"\n";
	output << "#define RAYLANG_MACRO_LINK_IMPORT\n";
	output << "#define RAYLANG_MACRO_LINK_EXPORT\n";
	output << "#define RAYLANG_MACRO_LINK_LOCAL\n";
	output << "#endif\n";
	std::string currentNamespace;
	std::string currentModule;
	for (const auto &symbol : symbolTable) {
		switch (symbol.type) {
		case analyzer::symbols::Symbol::SymbolType::Function: {
			if (auto *function =
			        dynamic_cast<const ast::Function *>(symbol.object)) {
				if (!function->publicVisibility) {
					output << "RAYLANG_MACRO_LINK_LOCAL ";
					output << "static ";
				} else if (function->body.has_value()) {
					if (function->body.has_value()) {
						output << "RAYLANG_MACRO_LINK_EXPORT ";
					} else {
						output << "RAYLANG_MACRO_LINK_IMPORT ";
					}
				}
				function->returnType.visit(*this);

				output << std::format("{}(", symbol.mangledName);
				for (size_t index = 0; index < function->params.size();
				     ++index) {
					const auto &parameter = function->params[index];
					parameter.visit(*this);
					if (index < function->params.size() - 1) {
						output << ", ";
					}
				}
				output << ");\n";
			}
			break;
		}
		case analyzer::symbols::Symbol::SymbolType::Struct: {
			if (auto *structObj =
			        dynamic_cast<const ast::Struct *>(symbol.object)) {
				output << std::format("{}typedef struct {}", currentIdent(),
				                      symbol.mangledName);
				if (!structObj->declaration) {
					output << " {\n";
					ident++;
					for (auto &member : structObj->members) {
						member.visit(*this);
					}
					ident--;
					output << std::format("{}}}", currentIdent());
				}
				output << std::format(" {};\n", symbol.mangledName);
			}
			break;
		}
		}
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

bool CTranspilerGenerator::hasFailed() const { return false; }

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
	std::string currentNamespace;
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
	std::string functionName = nameMangler.mangleFunction(
	    currentModule, currentNamespace, function, linkageDirective);

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
		std::cerr << std::format("'{}' is not a supported jump type\n",
		                         jump.keyword.getLexeme());
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
	std::string currentNamespace;
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
void CTranspilerGenerator::visitNamespaceStatement(const ast::Namespace &ns) {
	// Split ns.name by "::" and output each part
	std::string_view name = ns.name.getLexeme();
	auto delimiter = std::string_view("::");
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	size_t insertedNamespaces = 0;
	while ((pos_end = name.find(delimiter, pos_start)) != std::string::npos) {
		auto namespaceValue = name.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		this->namespaceStack.push_back(namespaceValue);
		insertedNamespaces++;
	}

	for (auto &value : ns.statements) {
		value->visit(*this);
	}
	while (insertedNamespaces-- > 0) {
		namespaceStack.pop_back();
	}
}
void CTranspilerGenerator::visitCompDirectiveStatement(
    const ast::CompDirective &value) {
	auto directiveName = value.name.getLexeme();
	if (directiveName == "Linkage") {
		auto &attributes = value.values;
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
		        : directive::LinkageDirective::ManglingType::Default);
		if (value.child) {
			auto childValue = value.child.get();
			if (dynamic_cast<ast::Function *>(childValue) ||
			    dynamic_cast<ast::Struct *>(childValue)) {
				size_t startDirectives = directivesStack.size();
				size_t originalTop = top + 1;
				top = startDirectives;
				directivesStack.push_back(
				    std::make_unique<directive::LinkageDirective>(directive));
				value.child->visit(*this);
				if (directivesStack.size() != startDirectives) {
					std::cerr << std::format(
					    "{}: unprocressed compiler directives.\n",
					    "COMPILER_BUG"_red, directive.directiveName());
				}
				top = originalTop;
			} else {
				std::cerr << std::format(
				    "{}: {} child expression must be a function or a struct.\n",
				    "ERROR"_red, directive.directiveName());
			}
		} else {
			std::cerr << std::format("{}: {} must have a child expression.\n",
			                         "ERROR"_red, directive.directiveName());
		}
	} else {
		std::cerr << std::format("{}: Unknown compiler directive '{}'.\n",
		                         "WARNING"_yellow, directiveName);
	}
}
// Expression
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
		std::cerr << std::format("'{}' is not a supported binary operation\n",
		                         op.getLexeme());
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
			std::cerr << std::format("{}: undefined symbol: '{}'\n",
			                         "ERROR"_red, var->name.lexeme);
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
	} else if (ast::Intrinsic *intr =
	               dynamic_cast<ast::Intrinsic *>(callable.callee.get())) {
		switch (intr->intrinsic) {
		case ray::compiler::ast::IntrinsicType::INTR_SIZEOF: {
			if (callable.arguments.size() != 1) {
				std::cerr << std::format("@sizeOf intrinsic expects 1 argument "
				                         "but {} got provided\n",
				                         callable.arguments.size());
			} else {
				auto param = callable.arguments[0].get();
				if (auto type = getTypeExpression(param)) {
					if (type->platformDependent) {
						output << std::format("((ssize)sizeof({}))",
						                      type->mangledName);
					} else {
						output
						    << std::format("((ssize){})", type->calculatedSize);
					}
				} else {
					std::cerr << std::format("'{}'is not a Type expression\n",
					                         param->variantName());
				}
			}
			break;
		}
		case ray::compiler::ast::IntrinsicType::INTR_UNKNOWN:
			std::cerr << std::format("'{}' is not a valid intrinsic\n",
			                         intr->name.lexeme);
			break;
		}
	} else {
		std::cerr << std::format("'{}' is not a supported callable type\n",
		                         callable.callee.get()->variantName());
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
		std::cerr << std::format("'{}' ({}) is not a supported literal type\n",
		                         literal.kind.getLexeme(),
		                         literal.kind.getGlyph());
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
		std::cerr << std::format("'{}' is not a supported unary operation\n",
		                         unary.op.getLexeme());
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
void CTranspilerGenerator::visitVariableExpression(
    const ast::Variable &variable) {
	output << std::format("{}", variable.name.lexeme);
}
void CTranspilerGenerator::visitIntrinsicExpression(
    const ast::Intrinsic &value) {
	std::cerr << "visitIntrinsicExpression not implemented\n";
}
void CTranspilerGenerator::visitTypeExpression(const ast::Type &type) {
	if (type.isConst && type.name.type != Token::TokenType::TOKEN_TYPE_UNIT &&
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
			std::cerr << std::format("{}: subtype null for array typke\n",
			                         "COMPILER_BUG"_red);
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
				std::cerr << std::format("{}: subtype null\n",
				                         "COMPILER_BUG"_red);
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

void CTranspilerGenerator::visitImportStatement(const ast::Import &value) {
	// the c transpiler does not need to either validate or verify this
}

std::string
CTranspilerGenerator::findCallableName(const ast::Call &callable,
                                       const std::string_view name) const {
	// we should make a propper lookup and ranking
	// but for now the first matching function will be used
	for (const auto &symbol : symbolTable) {
		if (symbol.name == name) {
			switch (symbol.type) {
			case analyzer::symbols::Symbol::SymbolType::Function: {
				if (const auto *function =
				        dynamic_cast<const ast::Function *>(symbol.object)) {
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
			case analyzer::symbols::Symbol::SymbolType::Struct: {
				if (const auto *currentStruct =
				        dynamic_cast<const ast::Struct *>(symbol.object)) {
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

std::optional<types::TypeInfo>
CTranspilerGenerator::findScalarTypeInfo(const std::string_view lexeme) {
	return types::TypeInfo::findScalarTypeInfo(lexeme);
}
std::optional<types::TypeInfo>
CTranspilerGenerator::findTypeInfo(const std::string_view lexeme) {
	auto scalarType = findScalarTypeInfo(lexeme);
	if (scalarType) {
		return scalarType;
	}
	return {};
}
std::optional<types::TypeInfo>
CTranspilerGenerator::getTypeExpression(const ast::Expression *expression) {
	if (auto var = dynamic_cast<const ast::Variable *>(expression)) {
		return findTypeInfo(var->name.lexeme);
	}
	return {};
}

} // namespace ray::compiler::generator::c

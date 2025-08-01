
#include <format>
#include <iostream>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/structDefinition.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/topLevelResolver.hpp>

namespace ray::compiler::analyzer {
using namespace terminal::literals;

void TopLevelResolver::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement) {
	currentS1SourceUnit.clear();
	for (const auto &stmt : statement) {
		stmt->visit(*this);
	}

	if (!this->directivesStack.empty()) {
		for (auto &directive : directivesStack) {
			std::cerr << std::format("{}: unused compiler directive {}\n",
			                         "WARNING"_yellow,
			                         directive->directiveName());
		}
	}
}

lang::S1SourceUnit TopLevelResolver::getSourceUnit() const {
	return currentS1SourceUnit;
}

bool TopLevelResolver::hasFailed() const { return messageBag.failed(); }
const std::vector<std::string> TopLevelResolver::getErrors() const {
	return messageBag.getErrors();
}
const std::vector<std::string> TopLevelResolver::getWarnings() const {
	return messageBag.getWarnings();
}

void TopLevelResolver::visitBlockStatement(const ast::Block &value) {
	// the top level resolver does not go inside definitionss
	// maybe a compiler directive could change this
	// ex: #[If()] directive
}
void TopLevelResolver::visitTerminalExprStatement(
    const ast::TerminalExpr &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitTerminalExprStatement not implemented");
}
void TopLevelResolver::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitExpressionStmtStatement not implemented");
}
void TopLevelResolver::visitFunctionStatement(const ast::Function &function) {
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
	std::string mangledFunctionName =
	    passes::mangling::NameMangler().mangleFunction(currentModule, function,
	                                                   linkageDirective);

	std::vector<std::string> signature = {function.returnType.name.lexeme};
	for (const auto &param : function.params) {
		signature.push_back(param.type.name.lexeme);
	}
	currentS1SourceUnit.rootScope.symbols.push_back(lang::S1Symbol{
		.type = lang::S1Symbol::SymbolType::Function,
		.name = std::string(function.name.getLexeme()),
		.signature = signature,
	});
	currentS1SourceUnit.functionDeclarations.push_back(
	    lang::S1FunctionDeclaration{
	        .name = std::string(function.name.getLexeme()),
	        .mangledName = mangledFunctionName,
	        .returnType = function.returnType.name.lexeme,
	    });
	if (function.body.has_value()) {
		function.body->visit(*this);
	}
}
void TopLevelResolver::visitIfStatement(const ast::If &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitIfStatement not implemented");
}
void TopLevelResolver::visitJumpStatement(const ast::Jump &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitJumpStatement not implemented");
}
void TopLevelResolver::visitVarStatement(const ast::Var &value) {
	// if we find a top level var statement check it as it might contain an
	// module import
	if (value.initializer) {
		value.initializer.value()->visit(*this);
	}
}
void TopLevelResolver::visitWhileStatement(const ast::While &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitWhileStatement not implemented");
}
void TopLevelResolver::visitStructStatement(const ast::Struct &structObj) {
	std::string currentModule;

	std::optional<directive::LinkageDirective> linkageDirective;

	for (size_t i = directivesStack.size(); i > top; i--) {
		auto &directive = directivesStack[i - i];
		if (auto foundLinkDirective =
		        dynamic_cast<directive::LinkageDirective *>(directive.get())) {
			linkageDirective = *foundLinkDirective;
		} else {
			std::cout << std::format(
			    "{}: unmatched compiler directive '{}' for struct.\n",
			    "WARNING"_yellow, directive->directiveName());
		}
		directivesStack.pop_back();
	}

	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(currentModule, structObj,
	                                                 linkageDirective);

	currentS1SourceUnit.structDeclarations.push_back(lang::S1StructDeclaration{
	    .name = std::string(structObj.name.getLexeme()),
	    .mangledName = mangledStructName,
	});
	if (!structObj.declaration) {
		std::vector<lang::S1StructMember> members;
		currentS1SourceUnit.structDefinitions.push_back(
		    lang::S1StructDefinition{
		        .name = std::string(structObj.name.getLexeme()),
		        .mangledName = mangledStructName,
		        .members = members,
		    });
	}
}
void TopLevelResolver::visitCompDirectiveStatement(
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
					messageBag.error(
					    childValue->getToken(), "BUG",
					    std::format("{}: unprocessed compiler directives.",
					                "BUG"_red));
				}
				top = originalTop;
			} else {
				messageBag.error(
				    childValue->getToken(), "RESOLVER",
				    std::format(
				        "{} child expression must be a function or a struct.",
				        directive.directiveName()));
			}
		} else {
			messageBag.error(compDirective.name, "RESOLVER",
			                 std::format("{} must have a child expression.",
			                             directive.directiveName()));
		}
	} else {
		messageBag.error(
		    compDirective.name, "RESOLVER",
		    std::format("Unknown compiler directive '{}'.", directiveName));
	}
}
// Expression
void TopLevelResolver::visitVariableExpression(const ast::Variable &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitVariableExpression not implemented");
}
void TopLevelResolver::visitIntrinsicExpression(const ast::Intrinsic &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitIntrinsicExpression not implemented");
}
void TopLevelResolver::visitAssignExpression(const ast::Assign &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitAssignExpression not implemented");
}
void TopLevelResolver::visitBinaryExpression(const ast::Binary &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitBinaryExpression not implemented");
}
void TopLevelResolver::visitCallExpression(const ast::Call &call) {
	// early top level scan does not require to evaluate call expressions
	// as modules and current intrinsics will be evaluated at a later pass
}
void TopLevelResolver::visitIntrinsicCallExpression(
    const ast::IntrinsicCall &value) {
	// early top level scan does not require to evaluate call expressions
	// as modules and current intrinsics will be evaluated at a later pass

	switch (value.callee->intrinsic) {
	case ast::IntrinsicType::INTR_SIZEOF:
	case ast::IntrinsicType::INTR_IMPORT:
		break;
	case ast::IntrinsicType::INTR_UNKNOWN: {
		messageBag.error(value.callee->name, "RESOLVER",
		                 std::format("unknown intrinsic type for '{}'",
		                             value.callee->name.getLexeme()));
		break;
	}
	}
}
void TopLevelResolver::visitGetExpression(const ast::Get &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitGetExpression not implemented");
}
void TopLevelResolver::visitGroupingExpression(const ast::Grouping &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitGroupingExpression not implemented");
}
void TopLevelResolver::visitLiteralExpression(const ast::Literal &literal) {}
void TopLevelResolver::visitLogicalExpression(const ast::Logical &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitLogicalExpression not implemented");
}
void TopLevelResolver::visitSetExpression(const ast::Set &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitSetExpression not implemented");
}
void TopLevelResolver::visitUnaryExpression(const ast::Unary &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitUnaryExpression not implemented");
}
void TopLevelResolver::visitArrayAccessExpression(
    const ast::ArrayAccess &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitArrayAccessExpression not implemented");
}
void TopLevelResolver::visitTypeExpression(const ast::Type &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitTypeExpression not implemented");
}
void TopLevelResolver::visitCastExpression(const ast::Cast &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitCastExpression not implemented");
}
void TopLevelResolver::visitParameterExpression(const ast::Parameter &value) {
	messageBag.error(value.getToken(), "BUG",
	                 "visitParameterExpression not implemented");
}

} // namespace ray::compiler::analyzer
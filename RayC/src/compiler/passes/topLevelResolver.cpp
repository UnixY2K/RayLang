#include <format>
#include <iostream>

#include <ray/cli/terminal.hpp>
#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/passes/symbol_mangler.hpp>
#include <ray/compiler/passes/topLevelResolver.hpp>

namespace ray::compiler::analyzer::symbols {
using namespace terminal::literals;

void TopLevelResolver::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement) {
	globalTable.clear();
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

SymbolTable TopLevelResolver::getSymbolTable() const { return globalTable; }

bool TopLevelResolver::hasFailed() const { return errorBag.failed(); }
const std::vector<std::string> TopLevelResolver::getErrors() const {
	return errorBag.getErrors();
}

void TopLevelResolver::visitBlockStatement(const ast::Block &value) {
	// the top level resolver does not go inside definitionss
	// maybe a compiler directive could change this
	// ex: #[If()] directive
}
void TopLevelResolver::visitTerminalExprStatement(
    const ast::TerminalExpr &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitTerminalExprStatement not implemented");
}
void TopLevelResolver::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitExpressionStmtStatement not implemented");
}
void TopLevelResolver::visitFunctionStatement(const ast::Function &function) {
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
	std::string mangledFunctionName =
	    passes::mangling::NameMangler().mangleFunction(
	        currentModule, currentNamespace, function, linkageDirective);
	globalTable.push_back(Symbol{
	    .name = std::string(function.name.getLexeme()),
	    .mangledName = mangledFunctionName,
	    .type = Symbol::SymbolType::Function,
	    .scope = currentNamespace,
	    .object = &function,
	});
	if (function.body.has_value()) {
		function.body->visit(*this);
	}
}
void TopLevelResolver::visitIfStatement(const ast::If &value) {
	errorBag.error(value.getToken(), "BUG", "visitIfStatement not implemented");
}
void TopLevelResolver::visitJumpStatement(const ast::Jump &value) {
	errorBag.error(value.getToken(), "BUG",
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
	errorBag.error(value.getToken(), "BUG",
	               "visitWhileStatement not implemented");
}
void TopLevelResolver::visitStructStatement(const ast::Struct &structObj) {
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
			    "{}: unmatched compiler directive '{}' for struct.\n",
			    "WARNING"_yellow, directive->directiveName());
		}
		directivesStack.pop_back();
	}

	std::string mangledStructName =
	    passes::mangling::NameMangler().mangleStruct(
	        currentModule, currentNamespace, structObj, linkageDirective);
	globalTable.push_back(Symbol{
	    .name = std::string(structObj.name.getLexeme()),
	    .mangledName = mangledStructName,
	    .type = Symbol::SymbolType::Struct,
	    .scope = currentNamespace,
	    .object = &structObj,
	});
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
		        : directive::LinkageDirective::ManglingType::Default);
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
					errorBag.error(
					    childValue->getToken(), "BUG",
					    std::format("{}: unprocessed compiler directives.",
					                "BUG"_red));
				}
				top = originalTop;
			} else {
				errorBag.error(
				    childValue->getToken(), "RESOLVER",
				    std::format(
				        "{} child expression must be a function or a struct.",
				        directive.directiveName()));
			}
		} else {
			errorBag.error(compDirective.name, "RESOLVER",
			               std::format("{} must have a child expression.",
			                           directive.directiveName()));
		}
	} else {
		errorBag.error(
		    compDirective.name, "RESOLVER",
		    std::format("Unknown compiler directive '{}'.", directiveName));
	}
}
// Expression
void TopLevelResolver::visitAssignExpression(const ast::Assign &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitAssignExpression not implemented");
}
void TopLevelResolver::visitBinaryExpression(const ast::Binary &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitBinaryExpression not implemented");
}
void TopLevelResolver::visitCallExpression(const ast::Call &call) {
	if (auto intrinsic = dynamic_cast<ast::Intrinsic *>(call.callee.get())) {
		switch (intrinsic->intrinsic) {
		case ast::IntrinsicType::INTR_SIZEOF: {
			break;
		}
		case ast::IntrinsicType::INTR_IMPORT: {
			if (call.arguments.size() != 1) {
				errorBag.error(
				    intrinsic->name, "RESOLVER",
				    std::format(
				        "@import requires 1 argument but {} were provided",
				        call.arguments.size()));
			}
			call.arguments[0]->visit(*this);
			if (evaluationStack.size() < 1) {
				errorBag.error(intrinsic->name, "BUG",
				               "evaluation stack underflow");
			} else {
				std::string filePath = evaluationStack.back();
				currentSourceUnit.requiredFiles.push_back(filePath);
			}
			break;
		}
		case ast::IntrinsicType::INTR_UNKNOWN: {
			errorBag.error(intrinsic->name, "RESOLVER",
			               std::format("unknown intrinsic type for '{}'",
			                           intrinsic->name.getLexeme()));
			break;
		}
		}
	} else {
		errorBag.error(call.callee->getToken(), "BUG",
		               std::format("call not supported for type {}",
		                           call.callee->variantName()));
	}
}
void TopLevelResolver::visitGetExpression(const ast::Get &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitGetExpression not implemented");
}
void TopLevelResolver::visitGroupingExpression(const ast::Grouping &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitGroupingExpression not implemented");
}
void TopLevelResolver::visitLiteralExpression(const ast::Literal &literal) {
	evaluationStack.push_back(literal.value);
}
void TopLevelResolver::visitLogicalExpression(const ast::Logical &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitLogicalExpression not implemented");
}
void TopLevelResolver::visitSetExpression(const ast::Set &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitSetExpression not implemented");
}
void TopLevelResolver::visitUnaryExpression(const ast::Unary &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitUnaryExpression not implemented");
}
void TopLevelResolver::visitArrayAccessExpression(
    const ast::ArrayAccess &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitArrayAccessExpression not implemented");
}
void TopLevelResolver::visitVariableExpression(const ast::Variable &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitVariableExpression not implemented");
}
void TopLevelResolver::visitIntrinsicExpression(const ast::Intrinsic &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitIntrinsicExpression not implemented");
}
void TopLevelResolver::visitTypeExpression(const ast::Type &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitTypeExpression not implemented");
}
void TopLevelResolver::visitCastExpression(const ast::Cast &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitCastExpression not implemented");
}
void TopLevelResolver::visitParameterExpression(const ast::Parameter &value) {
	errorBag.error(value.getToken(), "BUG",
	               "visitParameterExpression not implemented");
}

} // namespace ray::compiler::analyzer::symbols
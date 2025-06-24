#include <iostream>

#include <ray/cli/terminal.hpp>
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

bool TopLevelResolver::hasFailed() const { return false; }

void TopLevelResolver::visitBlockStatement(const ast::Block &value) {
	// the top level resolver does not go inside definitions
	// maybe a compiler directive could change this
	// ex: #[If()] directive
}
void TopLevelResolver::visitTerminalExprStatement(
    const ast::TerminalExpr &value) {
	std::cerr << "visitTerminalExprStatement not implemented\n";
}
void TopLevelResolver::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {
	std::cerr << "visitExpressionStmtStatement not implemented\n";
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
	std::cerr << "visitIfStatement not implemented\n";
}
void TopLevelResolver::visitJumpStatement(const ast::Jump &value) {
	std::cerr << "visitJumpStatement not implemented\n";
}
void TopLevelResolver::visitVarStatement(const ast::Var &value) {
	std::cerr << "visitVarStatement not implemented\n";
}
void TopLevelResolver::visitWhileStatement(const ast::While &value) {
	std::cerr << "visitWhileStatement not implemented\n";
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
void TopLevelResolver::visitNamespaceStatement(const ast::Namespace &ns) {
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
void TopLevelResolver::visitCompDirectiveStatement(
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
void TopLevelResolver::visitAssignExpression(const ast::Assign &value) {
	std::cerr << "visitAssignExpression not implemented\n";
}
void TopLevelResolver::visitBinaryExpression(const ast::Binary &value) {
	std::cerr << "visitBinaryExpression not implemented\n";
}
void TopLevelResolver::visitCallExpression(const ast::Call &value) {
	std::cerr << "visitCallExpression not implemented\n";
}
void TopLevelResolver::visitGetExpression(const ast::Get &value) {
	std::cerr << "visitGetExpression not implemented\n";
}
void TopLevelResolver::visitGroupingExpression(const ast::Grouping &value) {
	std::cerr << "visitGroupingExpression not implemented\n";
}
void TopLevelResolver::visitLiteralExpression(const ast::Literal &value) {
	std::cerr << "visitLiteralExpression not implemented\n";
}
void TopLevelResolver::visitLogicalExpression(const ast::Logical &value) {
	std::cerr << "visitLogicalExpression not implemented\n";
}
void TopLevelResolver::visitSetExpression(const ast::Set &value) {
	std::cerr << "visitSetExpression not implemented\n";
}
void TopLevelResolver::visitUnaryExpression(const ast::Unary &value) {
	std::cerr << "visitUnaryExpression not implemented\n";
}
void TopLevelResolver::visitArrayAccessExpression(
    const ast::ArrayAccess &value) {
	std::cerr << "visitArrayAccessExpression not implemented\n";
}
void TopLevelResolver::visitVariableExpression(const ast::Variable &value) {
	std::cerr << "visitVariableExpression not implemented\n";
}
void TopLevelResolver::visitIntrinsicExpression(const ast::Intrinsic &value) {
	std::cerr << "visitIntrinsicExpression not implemented\n";
}
void TopLevelResolver::visitTypeExpression(const ast::Type &value) {
	std::cerr << "visitTypeExpression not implemented\n";
}
void TopLevelResolver::visitCastExpression(const ast::Cast &value) {
	std::cerr << "visitCastExpression not implemented\n";
}
void TopLevelResolver::visitParameterExpression(const ast::Parameter &value) {
	std::cerr << "visitParameterExpression not implemented\n";
}

} // namespace ray::compiler::analyzer::symbols
#include <ray/compiler/generators/wasm_text.hpp>
#include <ray/compiler/lexer/token.hpp>

#include <format>
#include <iostream>
#include <string>

namespace ray::compiler::generator {

std::string WASMTextGenerator::currentIdent() const {
	return std::string(ident * 2, ' ');
}

void WASMTextGenerator::resolve(
    const std::vector<std::unique_ptr<ast::Statement>> &statement) {
	output.clear();
	output << "(module\n";
	ident++;
	for (const auto &stmt : statement) {
		stmt->visit(*this);
	}
	ident--;
	output << ")\n";
}

bool WASMTextGenerator::hasFailed() const { return false; }

std::string WASMTextGenerator::getOutput() const { return output.str(); }

// Statement
void WASMTextGenerator::visitBlockStatement(const ast::Block &block) {
	if (block.statements.size() > 0) {
		output << "\n";
		for (auto &statement : block.statements) {
			statement->visit(*this);
		}
	}
}
void WASMTextGenerator::visitTerminalExprStatement(
    const ast::TerminalExpr &terminalExpr) {
	if (terminalExpr.expression.has_value()) {
		terminalExpr.expression->get()->visit(*this);
		output << std::format("{}return\n", currentIdent());
	}
}
void WASMTextGenerator::visitExpressionStmtStatement(
    const ast::ExpressionStmt &value) {}
void WASMTextGenerator::visitFunctionStatement(const ast::Function &function) {

	std::string identTabs = currentIdent();
	std::string functionName;
	functionName.reserve(function.name.lexeme.size() + 12);
	functionName = function.name.lexeme;
	functionName = function.publicVisibility
	                   ? std::format("(export \"{}\")", functionName)
	                   : std::format("${}", functionName);
	output << identTabs;
	output << std::format("(func {}", functionName);
	for (auto &parameter : function.params) {
		output << std::format(" (param ${} {})", parameter.name.lexeme,
		                      parameter.type.name.lexeme);
	}
	if (function.returnType.name.type != Token::TokenType::TOKEN_TYPE_UNIT) {
		output << std::format(" (result {})", function.returnType.name.lexeme);
	}
	ident++;
	function.body.visit(*this);
	ident--;
	output << std::format("{})\n", identTabs);
}
void WASMTextGenerator::visitIfStatement(const ast::If &value) {
	std::cerr << "visitIfStatement not implemented\n";
}
void WASMTextGenerator::visitJumpStatement(const ast::Jump &value) {
	std::cerr << "visitJumpStatement not implemented\n";
}
void WASMTextGenerator::visitVarStatement(const ast::Var &value) {
	std::cerr << "visitVarStatement not implemented\n";
}
void WASMTextGenerator::visitWhileStatement(const ast::While &value) {
	std::cerr << "visitWhileStatement not implemented\n";
}
// Expression
void WASMTextGenerator::visitAssignExpression(const ast::Assign &value) {
	std::cerr << "visitAssignExpression not implemented\n";
}
void WASMTextGenerator::visitBinaryExpression(
    const ast::Binary &binaryExpression) {
	std::string identTab = currentIdent();
	binaryExpression.left->visit(*this);
	binaryExpression.right->visit(*this);
	auto op = binaryExpression.op;
	switch (op.type) {
	case Token::TokenType::TOKEN_PLUS:
		output << std::format("{}i32.add\n", identTab);
		break;
	default:
		std::cerr << std::format("'{}' is not a supported binary operation\n",
		                         op.getLexeme());
	}
}
void WASMTextGenerator::visitCallExpression(const ast::Call &value) {
	std::cerr << "visitCallExpression not implemented\n";
}
void WASMTextGenerator::visitGetExpression(const ast::Get &value) {
	std::cerr << "visitGetExpression not implemented\n";
}
void WASMTextGenerator::visitGroupingExpression(const ast::Grouping &value) {
	std::cerr << "visitGroupingExpression not implemented\n";
}
void WASMTextGenerator::visitLiteralExpression(const ast::Literal &value) {
	std::cerr << "visitLiteralExpression not implemented\n";
}
void WASMTextGenerator::visitLogicalExpression(const ast::Logical &value) {
	std::cerr << "visitLogicalExpression not implemented\n";
}
void WASMTextGenerator::visitSetExpression(const ast::Set &value) {
	std::cerr << "visitSetExpression not implemented\n";
}
void WASMTextGenerator::visitUnaryExpression(const ast::Unary &value) {
	std::cerr << "visitUnaryExpression not implemented\n";
}
void WASMTextGenerator::visitVariableExpression(const ast::Variable &value) {
	std::cerr << "visitVariableExpression not implemented\n";
}
void WASMTextGenerator::visitTypeExpression(const ast::Type &value) {
	std::cerr << "visitTypeExpression not implemented\n";
}
void WASMTextGenerator::visitParameterExpression(const ast::Parameter &value) {
	std::cerr << "visitParameterExpression not implemented\n";
}

} // namespace ray::compiler::generator

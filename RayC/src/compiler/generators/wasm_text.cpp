#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
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
    const ast::ExpressionStmt &expression) {
	expression.expression->visit(*this);
	output << std::format("{}drop\n", currentIdent());
}
void WASMTextGenerator::visitFunctionStatement(const ast::Function &function) {

	std::string identTabs = currentIdent();
	std::string functionName;
	functionName.reserve(function.name.lexeme.size() + 12);
	functionName = function.name.lexeme;
	output << identTabs;
	output << std::format("(func ${}", functionName);
	for (auto &parameter : function.params) {
		output << std::format(" (param ${} {})", parameter.name.lexeme,
		                      parameter.type.name.lexeme);
	}
	if (function.returnType.name.type != Token::TokenType::TOKEN_TYPE_UNIT) {
		output << std::format(" (result {})", function.returnType.name.lexeme);
	}
	output << "\n";
	ident++;
	function.body.visit(*this);
	ident--;
	output << std::format("{})\n", identTabs);
	if (function.publicVisibility) {
		output << std::format("{}(export \"{}\" (func ${}))\n", identTabs, functionName, functionName);
	}
}
void WASMTextGenerator::visitIfStatement(const ast::If &ifStatement) {
	ifStatement.condition->visit(*this);
	output << std::format("{}(if\n", currentIdent());
	ident++;
	output << std::format("{}(then\n", currentIdent());
	ident++;
	ifStatement.thenBranch->visit(*this);
	ident--;
	output << std::format("{})\n", currentIdent());
	if (ifStatement.elseBranch.has_value()) {
		output << std::format("{}else\n", currentIdent());
		ident++;
		ifStatement.elseBranch->get()->visit(*this);
		ident--;
	}
	ident--;
	output << std::format("{})\n", currentIdent());
}
void WASMTextGenerator::visitJumpStatement(const ast::Jump &jump) {
	std::string identTab = currentIdent();
	switch (jump.keyword.type) {
	case Token::TokenType::TOKEN_BREAK:
		output << std::format("{}br 0\n", identTab);
		break;
	case Token::TokenType::TOKEN_CONTINUE:
		output << std::format("{}br 1\n", identTab);
		break;
	case Token::TokenType::TOKEN_RETURN:
		jump.value->visit(*this);
		output << std::format("{}return\n", identTab);
		break;
	default:
		std::cerr << std::format("'{}' is not a supported jump type\n",
		                         jump.keyword.getLexeme());
		break;
	}
}
void WASMTextGenerator::visitVarStatement(const ast::Var &var) {
	std::string identTab = currentIdent();
	output << std::format("{}(local ${} i32)\n", identTab, var.name.lexeme);
	if (var.initializer.has_value()) {
		if (dynamic_cast<ast::Literal *>(var.initializer->get())) {
			output << std::format("{}(local.set ${} (", identTab,
			                      var.name.lexeme);
			auto currentIdent = ident;
			ident = 0;
			var.initializer.value()->visit(*this);
			ident = currentIdent;
			output << "))\n";
		} else {
			std::cerr << "visitVarStatement does not support non literal "
			             "initialization\n";
		}
	}
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
	case Token::TokenType::TOKEN_MINUS:
		output << std::format("{}i32.sub\n", identTab);
		break;
	case Token::TokenType::TOKEN_STAR:
		output << std::format("{}i32.mul\n", identTab);
		break;
	case Token::TokenType::TOKEN_SLASH:
		output << std::format("{}i32.div_s\n", identTab);
		break;
	case Token::TokenType::TOKEN_PERCENT:
		output << std::format("{}i32.rem_s\n", identTab);
		break;
	case Token::TokenType::TOKEN_AMPERSAND:
		output << std::format("{}i32.and\n", identTab);
		break;
	case Token::TokenType::TOKEN_PIPE:
		output << std::format("{}i32.or\n", identTab);
		break;
	case Token::TokenType::TOKEN_CARET:
		output << std::format("{}i32.xor\n", identTab);
		break;
	case Token::TokenType::TOKEN_LESS_LESS:
		output << std::format("{}i32.shl\n", identTab);
		break;
	case Token::TokenType::TOKEN_GREAT_GREAT:
		output << std::format("{}i32.shr\n", identTab);
		break;
	case Token::TokenType::TOKEN_LESS:
		output << std::format("{}i32.lt_s\n", identTab);
		break;
	case Token::TokenType::TOKEN_GREAT:
		output << std::format("{}i32.gt_s\n", identTab);
		break;
	case Token::TokenType::TOKEN_LESS_EQUAL:
		output << std::format("{}i32.le_s\n", identTab);
		break;
	case Token::TokenType::TOKEN_GREAT_EQUAL:
		output << std::format("{}i32.ge_s\n", identTab);
		break;
	default:
		std::cerr << std::format("'{}' is not a supported binary operation\n",
		                         op.getLexeme());
	}
}
void WASMTextGenerator::visitCallExpression(const ast::Call &callable) {
	for (auto &argument : callable.arguments) {
		argument->visit(*this);
	}
	// check if the callable contains a function
	if (ast::Variable *var =
	        dynamic_cast<ast::Variable *>(callable.callee.get())) {
		output << std::format("{}call ${}\n", currentIdent(), var->name.lexeme);
	} else {
		std::cerr << std::format("'{}' is not a supported callable type\n",
		                         callable.callee.get()->variantName());
	}
}
void WASMTextGenerator::visitGetExpression(const ast::Get &value) {
	std::cerr << "visitGetExpression not implemented\n";
}
void WASMTextGenerator::visitGroupingExpression(const ast::Grouping &grouping) {
	grouping.expression->visit(*this);
}
void WASMTextGenerator::visitLiteralExpression(const ast::Literal &literal) {
	switch (literal.kind.type) {
	case Token::TokenType::TOKEN_TRUE:
	case Token::TokenType::TOKEN_FALSE:
		output << std::format(
		    "{}i32.const {}", currentIdent(),
		    literal.kind.type == Token::TokenType::TOKEN_TRUE ? 1 : 0);
		break;
	case Token::TokenType::TOKEN_NUMBER: {
		std::string value = literal.value;
		output << std::format("{}i32.const {}", currentIdent(), value);
		break;
	}
	default:
		std::cerr << std::format("'{}' is not a supported literal type\n",
		                         literal.kind.getLexeme());
		break;
	}
	if (ident != 0) {
		output << "\n";
	}
}
void WASMTextGenerator::visitLogicalExpression(const ast::Logical &value) {
	std::cerr << "visitLogicalExpression not implemented\n";
}
void WASMTextGenerator::visitSetExpression(const ast::Set &value) {
	std::cerr << "visitSetExpression not implemented\n";
}
void WASMTextGenerator::visitUnaryExpression(const ast::Unary &unary) {
	unary.right->visit(*this);
	switch (unary.op.type) {
	case Token::TokenType::TOKEN_BANG:
		output << std::format("{}i32.eqz\n", currentIdent());
		break;
	case Token::TokenType::TOKEN_MINUS:
		output << std::format("{}i32.sub\n", currentIdent());
		break;
	case Token::TokenType::TOKEN_MINUS_MINUS:
		output << std::format("{}i32.const -1\n", currentIdent());
		output << std::format("{}i32.add\n", currentIdent());
		break;
	case Token::TokenType::TOKEN_PLUS_PLUS:
		output << std::format("{}i32.const 1\n", currentIdent());
		output << std::format("{}i32.sub\n", currentIdent());
		break;
	default:
		std::cerr << std::format("'{}' is not a supported unary operation\n",
		                         unary.op.getLexeme());
	}
}
void WASMTextGenerator::visitVariableExpression(const ast::Variable &variable) {
	std::string identTab = currentIdent();
	output << std::format("{}local.get ${}\n", identTab, variable.name.lexeme);
}
void WASMTextGenerator::visitTypeExpression(const ast::Type &value) {
	std::cerr << "visitTypeExpression not implemented\n";
}
void WASMTextGenerator::visitParameterExpression(const ast::Parameter &value) {
	std::cerr << "visitParameterExpression not implemented\n";
}

} // namespace ray::compiler::generator

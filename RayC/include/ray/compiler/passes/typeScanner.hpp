#pragma once

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/message_bag.hpp>

namespace ray::compiler::analyzer {
class TypeScanner : public ast::StatementVisitor,
                    public ast::ExpressionVisitor {
	MessageBag messageBag;

  public:
	TypeScanner(std::string filePath) : messageBag("TYPE-SCANNER", filePath) {}

	void resolve(const std::vector<std::unique_ptr<ast::Statement>> &statement);

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;
	const std::vector<std::string> getWarnings() const;
};
} // namespace ray::compiler::analyzer
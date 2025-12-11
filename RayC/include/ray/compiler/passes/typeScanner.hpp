#pragma once
#include <cstddef>
#include <unordered_map>

#include <ray/compiler/ast/expression.hpp>
#include <ray/compiler/ast/statement.hpp>
#include <ray/compiler/lang/type.hpp>
#include <ray/compiler/message_bag.hpp>

namespace ray::compiler::passes {
class TypeScanner : public ast::StatementVisitor,
                    public ast::ExpressionVisitor {
	MessageBag messageBag;

	std::unordered_map<size_t, lang::Type> typeData;
	std::unordered_map<size_t, ast::Struct> structData;

  public:
	TypeScanner(std::string filePath) : messageBag("TYPE-SCANNER", filePath) {}

	void resolve(const std::vector<std::unique_ptr<ast::Statement>> &statement);

	bool hasFailed() const;
	const std::vector<std::string> getErrors() const;
	const std::vector<std::string> getWarnings() const;
};
} // namespace ray::compiler::passes
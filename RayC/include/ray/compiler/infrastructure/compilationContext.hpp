#pragma once
#include <memory>

#include <optional>
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/infrastructure/diagnosticsEngine.hpp>
#include <ray/compiler/lang/module.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lexer/token.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>
#include <ray/compiler/syntax/rst/Statement.hpp>

namespace ray::compiler::infrastructure {
struct CompilationContext {
	lang::ModuleStore moduleStore;
	lang::SourceUnit sourceUnit;
	environment::DataModel dataModel;

	diagnostics::DiagnosticEngine diagnostics;
};

struct CompilerArtifact {
	std::optional<std::unique_ptr<syntax::ast::Block>> rootASTBlock = std::nullopt;
	std::optional<std::unique_ptr<syntax::rst::Block>> rootRSTBlock = std::nullopt;

	CompilerArtifact(
	    std::optional<std::unique_ptr<syntax::ast::Block>> rootASTBlock =
	        std::nullopt,
	    std::optional<std::unique_ptr<syntax::rst::Block>> rootRSTBlock =
	        std::nullopt)
	    : rootASTBlock(std::move(rootASTBlock)),
	      rootRSTBlock(std::move(rootRSTBlock)) {}

	CompilerArtifact(
	    std::optional<std::unique_ptr<syntax::rst::Block>> rootRSTBlock)
	    : rootRSTBlock(std::move(rootRSTBlock)) {}
};
} // namespace ray::compiler::infrastructure
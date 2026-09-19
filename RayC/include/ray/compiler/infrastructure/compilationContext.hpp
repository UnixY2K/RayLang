#pragma once
#include <ray/compiler/environment/dataModel/dataModel.hpp>
#include <ray/compiler/infrastructure/diagnosticsEngine.hpp>
#include <ray/compiler/lang/module.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/syntax/ast/Statement.hpp>

namespace ray::compiler::infrastructure {
struct CompilationContext {
	lang::ModuleStore moduleStore;
	lang::SourceUnit sourceUnit;
	environment::DataModel dataModel;

	diagnostics::DiagnosticEngine diagnostics;
};

struct CompilerArtifact {
	const syntax::ast::Block &rootASTBlock;
	const syntax::rst::Block &rootRSTBlock;
};
} // namespace ray::compiler::infrastructure
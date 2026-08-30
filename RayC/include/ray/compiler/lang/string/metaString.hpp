#pragma once

namespace ray::compiler::lang::string {
// a meta string is a intrinsic Staged abstract type designed arround
// comptime/eval time mostly used for Meta-Programming side of the language.
// examples:
// - intrinsic calls @Import("module.ray")
// - meta formatted strings "{moduleName}Rules.xyz"

class MetaString {};
} // namespace ray::compiler::lang::string
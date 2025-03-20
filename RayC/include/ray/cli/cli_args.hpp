#pragma once
#include <ray/cli/options.hpp>

#include <string>
#include <variant>
#include <vector>

namespace ray::compiler::cli {

std::variant<Options, std::vector<std::string>> parse_args(int argc,
                                                           char **argv);

} // namespace ray::compiler::cli

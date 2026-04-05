#pragma once
#include <expected>
#include <string>
#include <vector>

#include <ray/cli/options.hpp>

namespace ray::compiler::cli {

std::expected<Options, std::vector<std::string>> parse_args(int argc,
                                                            char **argv);

} // namespace ray::compiler::cli

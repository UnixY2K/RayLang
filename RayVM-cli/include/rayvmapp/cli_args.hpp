#pragma once
#include <rayvmapp/options.hpp>

#include <variant>

namespace ray::vmapp::terminal {

std::variant<Options, std::vector<std::string>> parse_args(int argc,
                                                           char **argv);

} // namespace ray::vmapp::terminal

#pragma once

#ifndef RAYC_APP_TERMINAL_COLOR_SUPPORT
// assume that terminal color support is not available by default
// meson will set it by default to true
#define RAYC_APP_TERMINAL_COLOR_SUPPORT false
#endif

namespace ray::compiler::definitions {
constexpr bool terminal_color_support = RAYC_APP_TERMINAL_COLOR_SUPPORT;
}

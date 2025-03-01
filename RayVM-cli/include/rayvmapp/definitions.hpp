#pragma once

#ifndef RAYVM_APP_TERMINAL_COLOR_SUPPORT
// assume that terminal color support is not available by default
// meson will set it by default to true
#define RAYVM_APP_TERMINAL_COLOR_SUPPORT false
#endif

namespace ray::vmapp::definitions {
constexpr bool terminal_color_support = RAYVM_APP_TERMINAL_COLOR_SUPPORT;
}

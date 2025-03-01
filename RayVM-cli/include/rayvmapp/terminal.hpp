#pragma once
#include <cstddef>
#include <string_view>

namespace ray::vmapp::terminal {
enum class Color { Red, Green, Blue, Yellow, White, Gray, Cyan, Orange, Lime };

std::string foreground_escape(Color color);
std::string colored(std::string_view str, Color color);

std::string red(std::string_view str);
std::string green(std::string_view str);
std::string blue(std::string_view str);
std::string yellow(std::string_view str);
std::string white(std::string_view str);
std::string gray(std::string_view str);
std::string cyan(std::string_view str);
std::string orange(std::string_view str);
std::string lime(std::string_view str);

namespace literals {
std::string operator""_red(const char *str, std::size_t len);
std::string operator""_green(const char *str, std::size_t len);
std::string operator""_blue(const char *str, std::size_t len);
std::string operator""_yellow(const char *str, std::size_t len);
std::string operator""_white(const char *str, std::size_t len);
std::string operator""_gray(const char *str, std::size_t len);
std::string operator""_cyan(const char *str, std::size_t len);
std::string operator""_orange(const char *str, std::size_t len);
std::string operator""_lime(const char *str, std::size_t len);
} // namespace literals
} // namespace ray::vmapp::terminal

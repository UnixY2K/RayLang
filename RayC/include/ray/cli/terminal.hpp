#pragma once
#include <cstddef>
#include <string>
#include <string_view>

#include <ray/cli/definitions.hpp>

namespace ray::compiler::terminal {
enum class Color {
	Red,
	Green,
	Blue,
	Yellow,
	White,
	Gray,
	Cyan,
	Orange,
	Lime,
	None
};

namespace esc_sequences {
constexpr std::string_view esc_reset = "\x1B[0m";
constexpr std::string_view esc_fg_red = "\x1B[31m";
constexpr std::string_view esc_fg_green = "\x1B[32m";
constexpr std::string_view esc_fg_blue = "\x1B[34m";
constexpr std::string_view esc_fg_yellow = "\x1B[33m";
constexpr std::string_view esc_fg_white = "\x1B[37m";
constexpr std::string_view esc_fg_gray = "\x1B[90m";
constexpr std::string_view esc_fg_cyan = "\x1B[36m";
constexpr std::string_view esc_fg_orange = "\x1B[38;5;202m";
constexpr std::string_view esc_fg_lime = "\x1B[38;5;112m";
constexpr std::string_view esc_fg_none = esc_reset;
} // namespace esc_sequences

constexpr std::string foreground_escape(Color color) {
	switch (color) {
	case Color::Red:
		return std::string(esc_sequences::esc_fg_red);
	case Color::Green:
		return std::string(esc_sequences::esc_fg_green);
	case Color::Blue:
		return std::string(esc_sequences::esc_fg_blue);
	case Color::Yellow:
		return std::string(esc_sequences::esc_fg_yellow);
	case Color::White:
		return std::string(esc_sequences::esc_fg_white);
	case Color::Gray:
		return std::string(esc_sequences::esc_fg_gray);
	case Color::Cyan:
		return std::string(esc_sequences::esc_fg_cyan);
	case Color::Orange:
		return std::string(esc_sequences::esc_fg_orange);
	case Color::Lime:
		return std::string(esc_sequences::esc_fg_lime);
	case Color::None:
		return std::string(esc_sequences::esc_fg_none);
	}
	return "";
}
constexpr std::string colored(std::string_view str, Color color) {
	if constexpr (definitions::terminal_color_support) {
		// we could use format, but it is a little overkill
		return foreground_escape(color) + std::string(str) +
		       std::string(esc_sequences::esc_reset);
	} else {
		return std::string(str);
	}
}

constexpr std::string red(std::string_view str) {
	return colored(str, Color::Red);
}
constexpr std::string green(std::string_view str) {
	return colored(str, Color::Green);
}
constexpr std::string blue(std::string_view str) {
	return colored(str, Color::Blue);
}
constexpr std::string yellow(std::string_view str) {
	return colored(str, Color::Yellow);
}
constexpr std::string white(std::string_view str) {
	return colored(str, Color::White);
}
constexpr std::string gray(std::string_view str) {
	return colored(str, Color::Gray);
}
constexpr std::string cyan(std::string_view str) {
	return colored(str, Color::Cyan);
}
constexpr std::string orange(std::string_view str) {
	return colored(str, Color::Orange);
}
constexpr std::string lime(std::string_view str) {
	return colored(str, Color::Lime);
}
constexpr std::string none(std::string_view str) {
	return colored(str, Color::None);
}

namespace literals {
constexpr std::string operator""_red(const char *str, std::size_t len) {
	return red(std::string_view(str, len));
}
constexpr std::string operator""_green(const char *str, std::size_t len) {
	return green(std::string_view(str, len));
}
constexpr std::string operator""_blue(const char *str, std::size_t len) {
	return blue(std::string_view(str, len));
}
constexpr std::string operator""_yellow(const char *str, std::size_t len) {
	return yellow(std::string_view(str, len));
}
constexpr std::string operator""_white(const char *str, std::size_t len) {
	return white(std::string_view(str, len));
}
constexpr std::string operator""_gray(const char *str, std::size_t len) {
	return gray(std::string_view(str, len));
}
constexpr std::string operator""_cyan(const char *str, std::size_t len) {
	return cyan(std::string_view(str, len));
}
constexpr std::string operator""_orange(const char *str, std::size_t len) {
	return orange(std::string_view(str, len));
}
constexpr std::string operator""_lime(const char *str, std::size_t len) {
	return lime(std::string_view(str, len));
}
constexpr std::string operator""_none(const char *str, std::size_t len) {
	return none(std::string_view(str, len));
}
} // namespace literals
} // namespace ray::compiler::terminal

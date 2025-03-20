#pragma once

#include <filesystem>

namespace ray::compiler::cli {

struct Options {
	bool assembly = false;
	bool disassembly = false;
	std::filesystem::path output;
	std::filesystem::path input;

	bool validate() const;
};
} // namespace ray::compiler::cli

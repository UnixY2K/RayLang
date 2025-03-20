#include <ray/cli/options.hpp>
#include <ray/cli/terminal.hpp>

#include <format>
#include <iostream>

namespace ray::compiler::cli {

using namespace terminal::literals;

bool Options::validate() const {
	bool success = true;
	if (assembly && disassembly) {
		std::cerr << std::format("{}: cannot specify both -S and -d\n",
		                         "Error"_red);
		success = false;
	}

	// check if the input file is a valid file
	if (!std::filesystem::exists(input)) {
		std::cerr << std::format("{}: input file '{}' does not exist\n",
		                         "Error"_red, input.string());
		success = false;
	} else if (std::filesystem::is_directory(input)) {
		std::cerr << std::format(
		    "{}: input file '{}' must be a file, not a directory\n",
		    "Error"_red, input.string());
		success = false;
	} else if (!std::filesystem::is_regular_file(input)) {
		std::cerr << std::format("{}: input file '{}' is not a regular file\n",
		                         "Error"_red, input.string());
		success = false;
	}

	// check if the output file is a valid location(not a path, or an exising
	// file)
	if (std::filesystem::exists(output)) {
		if (std::filesystem::is_directory(output)) {
			std::cerr << std::format(
			    "{}: output file '{}' must be a file, not a directory\n",
			    "Error"_red, output.string());
			success = false;
		} else if (!std::filesystem::is_regular_file(output)) {
			std::cerr << std::format(
			    "{}: output file '{}' is not a regular file\n", "Error"_red,
			    output.string());
			success = false;
		}
	}

	return success;
}

} // namespace ray::compiler::cli

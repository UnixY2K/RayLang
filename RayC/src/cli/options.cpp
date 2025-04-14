#include <ray/cli/options.hpp>
#include <ray/cli/terminal.hpp>

#include <format>
#include <iostream>
#include <string_view>
#include <unordered_map>

namespace ray::compiler::cli {

using namespace terminal::literals;

bool Options::validate() const {
	bool success = true;

	if (target == TargetEnum::ERROR) {
		std::cerr << std::format(
		    "{}: specifed target is not an existing target\n", "Error"_red);
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

Options::TargetEnum Options::targetFromString(std::string_view str) {
	static std::unordered_map<std::string, Options::TargetEnum> map{
	    {"none", Options::TargetEnum::NONE},
	    {"wasm_text", Options::TargetEnum::WASM_TEXT}};
	std::string key{str};
	std::transform(key.begin(), key.end(), key.begin(),
	               [](unsigned char c) { return std::tolower(c); });
	return map.contains(key) ? map.at(key) : Options::TargetEnum::ERROR;
}

} // namespace ray::compiler::cli

#include <expected>
#include <ray/cli/options.hpp>
#include <ray/cli/terminal.hpp>

#include <format>
#include <string_view>
#include <unordered_map>

namespace ray::compiler::cli {

using namespace terminal::literals;

std::expected<void, std::vector<std::string>> Options::validate() const {
	std::vector<std::string> errors;

	if (target == TargetEnum::ERROR) {
		errors.push_back(std::format(
		    "{}: specified target is not an existing target\n", "Error"_red));
	}

	// check if the input file is a valid file
	if (!std::filesystem::exists(input)) {
		errors.push_back(std::format("{}: input file '{}' does not exist\n",
		                             "Error"_red, input.string()));
	} else if (std::filesystem::is_directory(input)) {
		errors.push_back(
		    std::format("{}: input file '{}' must be a file, not a directory\n",
		                "Error"_red, input.string()));
	} else if (!std::filesystem::is_regular_file(input)) {
		errors.push_back(
		    std::format("{}: input file '{}' is not a regular file\n",
		                "Error"_red, input.string()));
	}

	// check if the output file is a valid location(not a path, or an exising
	// file)
	if (std::filesystem::exists(output)) {
		if (std::filesystem::is_directory(output)) {
			errors.push_back(std::format(
			    "{}: output file '{}' must be a file, not a directory\n",
			    "Error"_red, output.string()));
		} else if (!std::filesystem::is_regular_file(output)) {
			errors.push_back(
			    std::format("{}: output file '{}' is not a regular file\n",
			                "Error"_red, output.string()));
		}
	}

	if (errors.size()) {
		return std::unexpected(errors);
	}
	return {};
}

Options::TargetEnum Options::targetFromString(std::string_view str) {
	static std::unordered_map<std::string, Options::TargetEnum> map{
	    {"none", Options::TargetEnum::NONE},
	    {"c_source", Options::TargetEnum::C_SOURCE}};
	std::string key{str};
	std::transform(key.begin(), key.end(), key.begin(),
	               [](unsigned char c) { return std::tolower(c); });
	return map.contains(key) ? map.at(key) : Options::TargetEnum::ERROR;
}

} // namespace ray::compiler::cli

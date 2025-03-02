#include <rayvmapp/options.hpp>
#include <rayvmapp/terminal.hpp>

#include <cstddef>
#include <filesystem>
#include <format>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace ray::vmapp::terminal::literals;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::filesystem::path path = argv[0];
		std::cout << std::format(
		    "{}: {} {}\n", "Usage"_yellow,
		    ray::vmapp::terminal::lime(path.filename().string()),
		    "<binary file>"_cyan);
		return 1;
	}
	std::unordered_set<std::string> flags;
	std::unordered_map<std::string, std::string> options;
	std::string_view input_file;
	std::vector<std::string> options_stack;
	std::vector<std::string> errors;
	for (size_t i = 1; i < static_cast<size_t>(argc); i++) {
		std::string_view arg = argv[i];
		if (arg.starts_with("-") && arg.size() > 1) {
			char flag = arg[1];
			switch (flag) {
			case 'S': {
				flags.insert("assembly");
				break;
			}
			case 'd': {
				flags.insert("disassembly");
				break;
			}
			case 'o': {
				options_stack.push_back(std::string(arg));
				break;
			}
			default: {
				errors.push_back(
				    std::format("{}: unknown flag '{}'", "Error"_red, arg));
				break;
			}
			}
		} else {
			// common value
			if (!options_stack.empty()) {
				// consume the value
				options[options_stack.back()] = std::string(arg);
				options_stack.pop_back();
			} else {
				// take the value as the main input
				if (!input_file.empty()) {
					errors.push_back(
					    std::format("{}: only one input file can be specified",
					                "Error"_red));
					continue;
				}
				input_file = arg;
			}
		}
	}
	if (input_file.empty()) {
		errors.push_back(
		    std::format("{}: no input file specified", "Error"_red));
	}

	for (auto option : options_stack) {
		errors.push_back(std::format("{}: missing value for option '{}'",
		                             "Error"_red, option));
	}

	if (errors.size() > 0) {
		for (auto &error : errors) {
			std::cout << error << "\n";
		}
		return 1;
	}

	ray::vmapp::Options opts;
	opts.assembly = flags.contains("assembly");
	opts.disassembly = flags.contains("disassembly");
	opts.input = input_file;
	opts.output = options.contains("-o")
	                  ? options["-o"]
	                  : std::format("out.{}", opts.assembly ? "asm" : "bin");

	if (!opts.validate()) {
		return 1;
	}
}

#include <ray/cli/cli_args.hpp>
#include <ray/cli/options.hpp>
#include <ray/cli/terminal.hpp>

// non-windows platforms define some types inside of cstddef but windows
// already defined them so it can give a non used warning
#ifndef _MSC_VER
#include <cstddef>
#endif
#include <string_view>
#include <unordered_map>
#include <unordered_set>

using namespace ray::compiler::terminal::literals;

namespace ray::compiler::cli {

std::variant<Options, std::vector<std::string>> parse_args(int argc,
                                                           char **argv) {

	std::unordered_set<std::string> flags;
	std::unordered_map<std::string, std::string> options;
	std::string_view input_file;
	std::vector<std::string> options_stack;
	std::vector<std::string> errors;
	for (int i = 1; i < argc; i++) {
		std::string_view arg = argv[i];
		if (arg.starts_with("-") && arg.size() > 1) {
			char flag = arg[1];
			switch (flag) {
			case 'S': {
				flags.insert("assembly");
				break;
			}
			case 'o': {
				options_stack.push_back(std::string(arg));
				break;
			}
			case 't': {
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
		return errors;
	}

	Options opts;
	opts.assembly = flags.contains("assembly");
	opts.target = opts.targetFromString(
	    options.contains("-t") ? options.at("-t") : "none");
	opts.input = input_file;
	opts.output = options.contains("-o")
	                  ? options["-o"]
	                  : std::format("out.{}", opts.assembly ? "asm" : "bin");
	return opts;
}

} // namespace ray::compiler::cli

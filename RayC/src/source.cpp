#include <format>
#include <iostream>

#include <ray/cli/cli_args.hpp>

using namespace ray::compiler;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << std::format("Usage: {} <name>\n", argv[0]);
		return 1;
	}

	auto result = ray::compiler::cli::parse_args(argc, argv);
	if (std::holds_alternative<std::vector<std::string>>(result)) {
		for (const auto &error : std::get<std::vector<std::string>>(result)) {
			std::cout << error << '\n';
		}
		return 1;
	} else {
		auto opts = std::get<ray::compiler::cli::Options>(result);
		if (!opts.validate()) {
			return 1;
		}
	}
}

#include <rayvmapp/cli_args.hpp>
#include <rayvmapp/options.hpp>
#include <rayvmapp/terminal.hpp>

#include <ray/vm/assembler.hpp>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

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

	auto result = ray::vmapp::terminal::parse_args(argc, argv);
	if (std::holds_alternative<std::vector<std::string>>(result)) {
		for (const auto &error : std::get<std::vector<std::string>>(result)) {
			std::cout << error << '\n';
		}
		return 1;
	} else {
		auto opts = std::get<ray::vmapp::Options>(result);
		if (!opts.validate()) {
			return 1;
		}

		if (opts.assembly) {
			ray::vm::Assembler assembler;
			std::ifstream file(opts.input, std::ios::binary);
			if (!file.is_open()) {
				std::cerr << std::format("{}: Failed to open file '{}'\n",
				                         "Error"_red,
				                         opts.input.relative_path().string());
				return 1;
			}
			std::ostringstream buffer;
			buffer << file.rdbuf();
			auto result = assembler.assemble(buffer.view());
			if (std::holds_alternative<std::vector<std::string>>(result)) {
				for (const auto &error :
				     std::get<std::vector<std::string>>(result)) {
					std::cerr << std::format("{}: {}\n", "Error"_red, error);
				}
				return 1;
			} else {
				auto bytecode = std::get<std::vector<std::byte>>(result);
				std::ofstream output(opts.output, std::ios::binary);
				if (!output.is_open()) {
					std::cerr << std::format(
					    "{}: Failed to open file '{}'\n", "Error"_red,
					    opts.output.relative_path().string());
					return 1;
				}
				output.write(reinterpret_cast<const char *>(bytecode.data()),
				             bytecode.size());
			}
		}
	}
}

#include <rayvmapp/cli_args.hpp>
#include <rayvmapp/options.hpp>
#include <rayvmapp/terminal.hpp>

#include <filesystem>
#include <format>
#include <iostream>

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
	}
	else{
		auto opts = std::get<ray::vmapp::Options>(result);
		if (!opts.validate()) {
			return 1;
		}
	}
	
}

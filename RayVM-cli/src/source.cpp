#include <filesystem>
#include <format>
#include <iostream>

#include <rayvmapp/terminal.hpp>

using namespace ray::vmapp::terminal::literals;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::filesystem::path path = argv[0];
		std::cout << std::format(
		                 "{}: {} {}", "Usage"_yellow,
		                 ray::vmapp::terminal::lime(path.filename().string()),
		                 "<binary file>"_cyan)
		          << std::endl;
		return 1;
	}
}

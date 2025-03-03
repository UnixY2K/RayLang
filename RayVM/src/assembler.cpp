#include <ray/vm/assembler.hpp>

namespace ray::vm {

std::variant<std::vector<std::byte>, std::vector<std::string>>
Assembler::assemble(const std::string_view &source) {
	std::vector<std::byte> bytecode;
	std::vector<std::string> errors;

	if (!errors.empty()) {
		return errors;
	}
	return bytecode;
}

} // namespace ray::vm

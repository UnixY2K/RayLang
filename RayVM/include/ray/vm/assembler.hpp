#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <ray/vm/bytecode.hpp>

namespace ray::vm {

class Assembler {

	OpCode parse_opcode(const std::string_view token);
	std::tuple<std::vector<std::string>, bool>
	parse_operand(const std::string_view operand);

  public:
	[[nodiscard]]
	std::variant<std::vector<std::byte>, std::vector<std::string>>
	assemble(const std::string_view &source);
};

} // namespace ray::vm

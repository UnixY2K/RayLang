#pragma once
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace ray::vm {

class Assembler {
  public:
	[[nodiscard]]
	std::variant<std::vector<std::byte>, std::vector<std::string>>
	assemble(const std::string_view &source);
};

} // namespace ray::vm

#pragma once

#include <filesystem>
#include <string_view>

namespace ray::compiler::cli {

class Options {
  public:
	enum class TargetEnum {
		// default/explicitly set to none
		NONE,
		// output WebAssembly text format
		WASM_TEXT,
		ERROR,
	};

	bool assembly = false;
	TargetEnum target = TargetEnum::NONE;
	std::filesystem::path output;
	std::filesystem::path input;

	bool validate() const;

	static TargetEnum targetFromString(std::string_view str);
	static constexpr TargetEnum defaultTarget = TargetEnum::WASM_TEXT;
  private:
};
} // namespace ray::compiler::cli

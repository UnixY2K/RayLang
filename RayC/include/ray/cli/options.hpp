#pragma once

#include <filesystem>
#include <string_view>

namespace ray::compiler::cli {

class Options {
  public:
	enum class TargetEnum {
		// default/explicitly set to none
		NONE,
		// output a combination of c header and unit with the same name
		C_SOURCE,
		// used when there is no matching equivalent of the requested option
		ERROR,
	};

	bool assembly = false;
	TargetEnum target = TargetEnum::NONE;
	std::filesystem::path output;
	std::filesystem::path input;

	bool validate() const;

	static TargetEnum targetFromString(std::string_view str);
	static constexpr TargetEnum defaultTarget = TargetEnum::C_SOURCE;

  private:
};
} // namespace ray::compiler::cli

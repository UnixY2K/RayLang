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

	enum class TargetDataModel {
		// default
		NONE,
		// x32, arm64ilp32 on Linux; MIPS N32 ABI
		ILP32,
		// (x86-64, IA-64, and ARM64) MSVC/MinGW
		LLP64,
		// Most Unix/Unix-like systems: (Solaris, Linux, BSD, macOS);
		// Cygwin; z/OS
		LP64,
		// HAL Computer Systems port of Solaris to the SPARC64
		ILP64,
		// Classic UNICOS (versus UNICOS/mp, etc.)
		SILP64
	};

	bool assembly = false;
	TargetEnum target = TargetEnum::NONE;
	std::filesystem::path output;
	std::filesystem::path input;
	TargetDataModel dataModel = getHostDataModel();

	bool validate() const;

	static TargetEnum targetFromString(std::string_view str);
	static constexpr TargetEnum defaultTarget = TargetEnum::C_SOURCE;
	static constexpr TargetDataModel getHostDataModel() {
// TODO: add propper setup for Windows, MacOS and Linux
#ifdef __linux__
#ifdef __LP64__
		return TargetDataModel::LP64;
#else // assume 32 bits
#endif
#else
#warning "non supported host arch"
		return TargetDataModel::NONE;
#endif
	}

  private:
};
} // namespace ray::compiler::cli

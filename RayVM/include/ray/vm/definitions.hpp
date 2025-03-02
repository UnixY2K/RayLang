#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace ray::vm::definitions {
using Byte = std::byte;
union Word {
	std::array<Byte, 2> bytes;
	uint16_t word;
	struct {
		Byte low;
		Byte high;
	};
};
union DWord {
	std::array<Byte, 4> bytes;
	uint32_t dword;
	struct {
		Word low;
		Word high;
	};
};
union QWord {
	std::array<Byte, 8> bytes;
	uint64_t qword;
	struct {
		DWord low;
		DWord high;
	};
};

} // namespace ray::vm::definitions

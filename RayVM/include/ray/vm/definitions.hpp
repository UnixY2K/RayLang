#pragma once

#include <cstddef>
#include <cstdint>

namespace ray::vm::definitions {
using Byte = std::byte;
union Word {
	uint16_t word;
	struct {
		Byte low;
		Byte high;
	};
};
union DWord {
	uint32_t dword;
	struct {
		Word low;
		Word high;
	};
};
union QWord {
	uint64_t qword;
	struct {
		DWord low;
		DWord high;
	};
};

} // namespace ray::vm::defintions

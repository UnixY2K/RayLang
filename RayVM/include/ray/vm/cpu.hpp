#pragma once

#include <array>
#include <ray/vm/definitions.hpp>

namespace ray::vm {

class CPUCore {
  public:
	// general purpose registers
	std::array<definitions::QWord, 16> gpr;
	// stack pointer
	uint64_t sp = 0;
	// program counter/ instruction pointer
	uint64_t pc = 0;
	// return address
	uint64_t ra = 0;

	// trap flags
	// zero flag
	bool zf = false;
	// overflow flag
	bool of = false;
	// invalid address flag
	bool inv_addr_f = false;
	// invalid instruction flag
	bool inv_inst_f = false;
};
} // namespace ray::vm

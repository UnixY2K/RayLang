#pragma once

#include <cstddef>
#include <ray/vm/cpu.hpp>
#include <ray/vm/definitions.hpp>
#include <ray/vm/memory.hpp>

namespace ray::vm {

class VM {
	CPUCore cpu;
	Memory memory;

  public:
	VM(std::size_t memory_size = 256);

	void load_program(const std::vector<std::byte> &program);

	void run();

  private:
	void fetch();
	void decode();
	void execute();
};

} // namespace ray::vm

#pragma once

#include <array>
#include <string>

namespace ray::vm {
enum class OpCode {
	// Arithmetic
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	// Comparison
	Eq,
	Neq,
	Lt,
	Lte,
	Gt,
	Gte,
	// Logical
	And,
	Or,
	Not,
	// Control flow
	Beq,
	JmpIf,
	JmpIfNot,
	Call,
	Ret,
	// Memory
	Load,
	Store,
	// Misc
	Nop,
	Halt,
	// invalid/non parsable
	INVALID,
};

struct Instruction {
	OpCode opcode;
	std::array<std::string, 3> operands;
};

} // namespace ray::vm

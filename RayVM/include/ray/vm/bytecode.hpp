#pragma once

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
	Jmp,
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
};
}

#include <algorithm>
#include <cstddef>
#include <format>
#include <iostream>
#include <unordered_map>

#include <ray/vm/assembler.hpp>

namespace ray::vm {

OpCode Assembler::parse_opcode(const std::string_view token) {
	static const std::unordered_map<std::string_view, OpCode> opcodes = {
	    {"add", OpCode::Add},
	    {"sub", OpCode::Sub},
	    {"mul", OpCode::Mul},
	    {"div", OpCode::Div},
	    {"mod", OpCode::Mod},
	    {"eq", OpCode::Eq},
	    {"neq", OpCode::Neq},
	    {"lt", OpCode::Lt},
	    {"lte", OpCode::Lte},
	    {"gt", OpCode::Gt},
	    {"gte", OpCode::Gte},
	    {"and", OpCode::And},
	    {"or", OpCode::Or},
	    {"not", OpCode::Not},
	    {"jmp", OpCode::Beq},
	    {"jmpif", OpCode::JmpIf},
	    {"jmpifnot", OpCode::JmpIfNot},
	    {"call", OpCode::Call},
	    {"ret", OpCode::Ret},
	    {"load", OpCode::Load},
	    {"store", OpCode::Store},
	    {"nop", OpCode::Nop},
	    {"halt", OpCode::Halt},
	};
	std::string lower_token(token);
	std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(),
	               [](unsigned char c) { return std::tolower(c); });

	auto it = opcodes.find(lower_token);
	return it != opcodes.end() ? it->second : OpCode::INVALID;
}

std::tuple<std::vector<std::string>, bool>
Assembler::parse_operand(const std::string_view operands_sv) {
	bool valid = true;
	std::vector<std::string> operands;
	std::vector<std::string> errors;

	for (size_t i = 0; i < operands_sv.size(); ++i) {
		// split by comma
		size_t start = i;
		while (i < operands_sv.size() && operands_sv[i] != ',') {
			++i;
		}
		std::string_view operand = operands_sv.substr(start, i - start);
		// remove leading and trailing spaces
		size_t start_ws = 0;
		size_t end_ws = operand.size();
		while (start_ws < end_ws && std::isspace(operand[start_ws])) {
			++start_ws;
		}
		while (end_ws > start_ws && std::isspace(operand[end_ws - 1])) {
			--end_ws;
		}
		operand = operand.substr(start_ws, end_ws - start_ws);
		if (operand.empty()) {
			errors.push_back("Empty operand");
			valid = false;
		} else {
			operands.push_back(std::string(operand));
		}
	}

	return {valid ? operands : errors, valid};
}

[[nodiscard]]

std::variant<std::vector<std::byte>, std::vector<std::string>>
Assembler::assemble(const std::string_view &source) {
	std::vector<std::byte> bytecode;
	std::vector<std::string> errors;
	std::unordered_map<std::string, size_t> tags;
	std::vector<Instruction> instructions;

	size_t line = 1;
	size_t column = 1;

	for (size_t i = 0; i < source.size(); ++i) {
		switch (source[i]) {
		case '\n':
			++line;
			column = 1;
			break;
		case ' ':
		case '\t':
			++column;
			break;
		case ';':
			while (i < source.size() && source[i] != '\n') {
				++i;
			}
			break;
		default:
			// possible instruction or tag
			size_t start = i;
			while (i < source.size() && source[i] != ' ' && source[i] != '\t' &&
			       source[i] != '\n') {
				++i;
				line += source[i] == '\n';
				column = source[i] == '\n' ? 1 : column + 1;
			}
			std::string_view token = source.substr(start, i - start);

			// check if it's a tag
			if (token.back() == ':') {
				token.remove_suffix(1);
				std::string tag = std::string(token);
				if (tags.contains(tag)) {
					errors.push_back(
					    std::format("Duplicate tag '{}' at line {} column {}",
					                tag, line, column));
					break;
				} else {
					tags.emplace(tag, bytecode.size());
				}
			} else {
				OpCode opcode = parse_opcode(token);
				if (opcode == OpCode::INVALID) {
					errors.push_back(
					    std::format("Invalid opcode '{}' at line {} column {}",
					                token, line, column));
				} else {
					// read until first non-space/tab character
					while (i < source.size() &&
					       (source[i] == ' ' || source[i] == '\t')) {
						++i;
						column++;
					}
					size_t start = i;
					// read until end of line or comment
					while (i < source.size() && source[i] != '\n' &&
					       source[i] != ';') {
						++i;
						column += source[i] == '\n';
					}
					size_t end = i;
					std::string_view operands_sv =
					    source.substr(start, end - start);
					auto [list, valid] = parse_operand(operands_sv);
					if (!valid) {
						errors.push_back(std::format(
						    "Invalid operands '{}' at line {} column {}",
						    operands_sv, line, column));
						// the list contains the error message
						for (const auto &error : list) {
							errors.push_back(error);
						}
					}

					// write instruction with opcode and operands
					Instruction instr{opcode};
					if (list.size() > instr.operands.size()) {
						errors.push_back(
						    std::format("Too many operands for opcode '{}' at "
						                "line {} column {}",
						                token, line, column));
					} else {
						for (size_t i = 0; i < list.size(); ++i) {
							instr.operands[i] = list[i];
						}
					}
				}
				// read until end of line
				while (i < source.size() && source[i] != '\n') {
					++i;
				}
				line += source[i] == '\n';
				column = source[i] == '\n' ? 1 : column + 1;
			}
			break;
		}
	}

	if (!errors.empty()) {
		return errors;
	}
	return bytecode;
}

} // namespace ray::vm

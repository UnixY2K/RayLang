#pragma once

#include <ray/vm/definitions.hpp>

#include <vector>

namespace ray::vm {

class Memory {

	using Byte = definitions::Byte;
	using Word = definitions::Word;
	using DWord = definitions::DWord;
	using QWord = definitions::QWord;

	std::vector<Byte> data;

  public:
	void resize(std::size_t size) { data.resize(size); }

	Byte readByte(std::size_t index) const;
	bool writeByte(std::size_t index, Byte value);

	Word readWord(std::size_t index) const;
	bool writeWord(std::size_t index, Word value);

	DWord readDWord(std::size_t index) const;
	bool writeDWord(std::size_t index, DWord value);

	QWord readQWord(std::size_t index) const;
	bool writeQWord(std::size_t index, QWord value);

	std::size_t size() const { return data.size(); }
};
} // namespace ray::vm

#include <ray/vm/memory.hpp>

namespace ray::vm {

using Byte = definitions::Byte;
using Word = definitions::Word;
using DWord = definitions::DWord;
using QWord = definitions::QWord;

void Memory::resize(std::size_t size) { data.resize(size); }

Byte Memory::readByte(std::size_t index) const {
	if (index >= size()) {
		return static_cast<Byte>(0);
	}
	return data.at(index);
}
bool Memory::writeByte(std::size_t index, Byte value) {
	if (index >= size()) {
		return false;
	}
	data.at(index) = value;
	return true;
}

Word Memory::readWord(std::size_t index) const {
	Word word = {};
	for (std::size_t i = 0; i < std::min(sizeof(Word), size() - index); i++) {
		word.bytes.at(i) = data.at(index + i);
	}
	return word;
}
bool Memory::writeWord(std::size_t index, Word value) {
	if (index > size() - sizeof(Word)) {
		return false;
	}
	for (std::size_t i = 0; i < sizeof(Word); i++) {
		data.at(index + i) = value.bytes.at(i);
	}
	return true;
}

DWord Memory::readDWord(std::size_t index) const {
	DWord dword = {};
	for (std::size_t i = 0; i < std::min(sizeof(DWord), size() - index); i++) {
		dword.bytes.at(i) = data.at(index + i);
	}
	return dword;
}
bool Memory::writeDWord(std::size_t index, DWord value) {
	if (index > size() - sizeof(DWord)) {
		return false;
	}
	for (std::size_t i = 0; i < sizeof(DWord); i++) {
		data.at(index + i) = value.bytes.at(i);
	}
	return true;
}

QWord Memory::readQWord(std::size_t index) const {
	QWord qword = {};
	for (std::size_t i = 0; i < std::min(sizeof(QWord), size() - index); i++) {
		qword.bytes.at(i) = data.at(index + i);
	}
	return qword;
}
bool Memory::writeQWord(std::size_t index, QWord value) {
	if (index > size() - sizeof(QWord)) {
		return false;
	}
	for (std::size_t i = 0; i < sizeof(QWord); i++) {
		data.at(index + i) = value.bytes.at(i);
	}
	return true;
}

std::size_t Memory::size() const { return data.size(); }

} // namespace ray::vm

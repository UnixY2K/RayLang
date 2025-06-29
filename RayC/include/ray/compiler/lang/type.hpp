#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>

namespace ray::compiler::lang {
class Type {

	bool initialized = false;
	bool scalar = false;

	void copy(const Type &other) {
		initialized = other.initialized;
		scalar = other.initialized;

		name = other.name;
		isConst = other.isConst;
		isPointer = other.isPointer;

		if (other.subtype) {
			auto newSubtype = *other.subtype->get();
			subtype = std::make_unique<Type>(newSubtype);
		}
	}

  public:
	Type() = default;
	Type(const Type &other) { copy(other); }

	std::string name;
	size_t requiredBytes = 0;
	bool isConst = true;
	bool isPointer = false;
	std::optional<std::unique_ptr<Type>> subtype;

	void initialize() { initialized = true; }
	bool isInitialized() const { return initialized; }
	bool isScalar() const { return scalar; }

	Type &operator=(const Type &other) {
		copy(other);
		return *this;
	}
};
} // namespace ray::compiler::lang
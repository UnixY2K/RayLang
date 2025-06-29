#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>

namespace ray::compiler::lang {
class Type {

	bool initialized = false;
	bool scalar = false;

  public:
	std::string name;
	size_t requiredBytes = 0;
	bool isConst = true;
	bool isPointer = false;
	std::optional<std::unique_ptr<Type>> subtype;

	bool isInitialized() const { return initialized; }
	bool isScalar() const { return scalar; }
};
} // namespace ray::compiler::lang
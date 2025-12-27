#pragma once
#include <cstddef>
#include <functional>
#include <optional>

namespace ray::compiler::util {
template <typename T> class soft_reference {
	size_t objectId;
	// the lifetime of the object has to be longer
	// than our reference
	std::optional<std::reference_wrapper<const T>> object;

  public:
	soft_reference(const size_t objectId = 0,
	               const std::optional<std::reference_wrapper<const T>> object =
	                   std::nullopt)
	    : objectId(objectId), object(object) {}

	size_t getObjectId() const { return objectId; }
	void setObjectId(const size_t objectId) { this->objectId = objectId; }

	const std::optional<std::reference_wrapper<const T>> getObject() const {
		return object;
	}

	void setObject(const T &object) { this->object = object; }
};

} // namespace ray::compiler::util
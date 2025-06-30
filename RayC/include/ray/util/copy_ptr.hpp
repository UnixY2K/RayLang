#pragma once
#include <concepts>
#include <memory>

namespace ray::compiler::util {
template <typename T> class copy_ptr {
	std::unique_ptr<T> ptr;

  public:
	copy_ptr() = default;

	copy_ptr(const copy_ptr<T> &other)
		: ptr{other.ptr ? std::make_unique<T>(*other.ptr) : nullptr} {}

	copy_ptr(copy_ptr<T> &&other) noexcept = default;

	copy_ptr<T> &operator=(const copy_ptr<T> &other) {
		if (this != &other) {
			ptr = other.ptr ? std::make_unique<T>(*other.ptr) : nullptr;
		}
		return *this;
	}

	copy_ptr<T> &operator=(copy_ptr<T> &&other) noexcept = default;

	explicit copy_ptr(T *raw_ptr) : ptr(raw_ptr) {}

	explicit copy_ptr(std::unique_ptr<T> &&unique_ptr) : ptr(std::move(unique_ptr)) {}

	T &operator*() const { return *ptr; }
	T *operator->() const { return ptr.get(); }
	T *get() const { return ptr.get(); }
	explicit operator bool() const { return static_cast<bool>(ptr); }

	void reset(T *raw_ptr = nullptr) { ptr.reset(raw_ptr); }
	void swap(copy_ptr &other) noexcept { ptr.swap(other.ptr); }
};
} // namespace ray::compiler::util
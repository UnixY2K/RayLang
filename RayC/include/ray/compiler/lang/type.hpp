#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <ray/util/copy_ptr.hpp>

namespace ray::compiler::lang {
class Type;
class Type {

	bool initialized = false;
	bool scalar = false;

  public:
	// an internal id used to track additional type information
	// such as struct or function data
	size_t typeId;
	std::string name;
	size_t calculatedSize = 0;
	bool isMutable = false;
	bool isPointer = false;
	bool signedType = false;
	bool overloaded = false;
	std::optional<util::copy_ptr<Type>> subtype = std::nullopt;
	std::optional<std::vector<util::copy_ptr<Type>>> signature = std::nullopt;

	Type() = default;
	Type(bool initialized, bool scalar, std::string name, size_t calculatedSize,
	     bool isMutable, bool isPointer, bool signedType, bool overloaded,
	     std::optional<util::copy_ptr<Type>> subType,
	     std::optional<std::vector<util::copy_ptr<Type>>> signature)
	    : initialized{initialized}, scalar{scalar}, name{name},
	      calculatedSize{calculatedSize}, isMutable{isMutable},
	      isPointer{isPointer}, signedType{signedType}, overloaded{overloaded},
	      subtype{subType}, signature{signature} {};

	void initialize() { initialized = true; }
	bool isInitialized() const { return initialized; }
	bool isScalar() const { return scalar; }

	bool coercercesInto(const Type &targetType) const;
	bool signatureEquals(const Type &targetType) const;
	bool signatureMatches(const Type &targetType) const;

	bool operator==(const Type &other) const;

	// returns a type that is not instatiable and cannot be used
	// used by statements in the type checker
	static constexpr Type defineStmtType() {
		return Type{
		    // an statement does not even return an initialized type
		    false,
		    false, // non scalar
		    // name cannot be mangled nor referenced
		    "%<stmt>%",
		    // size is 0 so it cannot be passed
		    0,
		    false, // non mutable
		    false, // non pointer
		    false, // non signed
		    false, // non overloaded
		    {},    // no subtype data
		    {},    // no signature data
		};
	}

	// defines an unknown type which is used when a child expression searched
	// for a valid type but did not found a matching value
	static constexpr Type defineUnknownType() {
		return Type{
		    // an statement does not even return an initialized type
		    false,
		    false, // non scalar
		    // name cannot be mangled nor referenced
		    "%<unknown>%",
		    // size is 0 so it cannot be passed
		    0,
		    false, // non mutable
		    false, // non pointer
		    false, // non signed
		    false, // non overloaded
		    {},    // no subtype data
		    {},    // no signature data
		};
	}

	// defines an empty module type used by the type checker
	static constexpr Type defineModuleType() {
		return Type{
		    // an statement does not even return an initialized type
		    true,
		    false, // non scalar
		    // name cannot be mangled nor referenced
		    "%<module>%",
		    // size is 0 so it cannot be passed
		    0,
		    false, // non mutable
		    false, // non pointer
		    false, // non signed
		    false, // non overloaded
		    {},    // no subtype data
		    {},    // no signature data
		};
	}

  private:
	bool baseMatches(const Type &other) const;
};
} // namespace ray::compiler::lang
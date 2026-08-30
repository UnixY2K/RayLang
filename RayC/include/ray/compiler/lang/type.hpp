#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <ray/util/copy_ptr.hpp>

namespace ray::compiler::lang {

enum class TypeKind { scalar, aggregate, pointer, abstract };
class Type;
class Type {

	bool initialized = false;
	TypeKind kind = TypeKind::abstract;

  public:
	// an internal id used to track additional type information
	// such as struct or function data
	size_t typeId = 0;
	std::string name;
	size_t calculatedSize = 0;
	bool isMutable = false;
	bool signedType = false;
	bool overloaded = false;
	// TODO: make both the subtype and signature hold a soft_reference instead
	// this would help to avoid duplicates of the same type while
	// allowing for recursive types such as function pointers
	std::optional<util::copy_ptr<Type>> subtype = std::nullopt;
	std::optional<std::vector<util::copy_ptr<Type>>> signature = std::nullopt;

	Type() = default;
	Type(const size_t typeId, const bool initialized, const TypeKind kind,
	     const std::string name, const size_t calculatedSize,
	     const bool isMutable, const bool signedType, const bool overloaded,
	     std::optional<util::copy_ptr<Type>> subType,
	     std::optional<std::vector<util::copy_ptr<Type>>> signature)
	    : initialized{initialized}, kind{kind}, typeId(typeId), name{name},
	      calculatedSize{calculatedSize}, isMutable{isMutable},
	      signedType{signedType}, overloaded{overloaded}, subtype{subType},
	      signature{signature} {};

	void initialize() { initialized = true; }
	bool isInitialized() const { return initialized; }
	TypeKind getKind() const { return kind; }

	bool coercercesInto(const Type &targetType) const;
	// checks for strict equality between types
	bool signatureEquals(const Type &targetType) const;
	// checks for a match in the signature considering type coercion
	bool signatureMatches(const Type &targetType) const;

	bool operator==(const Type &other) const;

	// returns a type that is not instatiable and cannot be used
	// used by statements in the type checker
	static constexpr Type defineStmtType() {
		return defineNamedAbstractType("%<stmt>%");
	}

	// defines an unknown type which is used when a child expression searched
	// for a valid type but did not found a matching value
	static constexpr Type defineUnknownType() {
		return defineNamedAbstractType("%<unknown>%");
	}

	// defines an empty module type used by the type checker
	static constexpr Type defineModuleType() {
		return defineNamedAbstractType("%<module>%");
	}

	// defines a "meta Type" Type, which is basically and abstract type
	// that holds a Type information, ex: @sizeOf(c_char) where c_char is the
	// meta type passed to the intrinsic
	static constexpr Type defineMetaTypeType() {
		return defineNamedAbstractType("%<Type>%");
	}

	// defines a known metaString type
	static constexpr Type defineMetaStringType() {
		return defineNamedAbstractType("%<MetaString>%");
	}

	// defines an intrinsic expression, ex: @import
	// note: its result must be evaluated later, this only exposes the intrinsic
	// itself as a Type
	static constexpr Type defineIntrinsicType(
	    const std::string &name,
	    // subtype(return type of the intrinsic(if known))
	    // signature of the intrinsic to validate(if known)
	    std::optional<util::copy_ptr<Type>> subType = std::nullopt,
	    std::optional<std::vector<util::copy_ptr<Type>>> signature =
	        std::nullopt) {
		return defineNamedAbstractType(
		    "%<intrinsic>%", defineNamedAbstractType(name, subType, signature));
	}

	static constexpr Type defineNamedAbstractType(
	    const std::string &name,
	    std::optional<util::copy_ptr<Type>> subType = std::nullopt,
	    std::optional<std::vector<util::copy_ptr<Type>>> signature =
	        std::nullopt) {
		return Type{
		    // named abstract do not have a defined typeID
		    0,
		    // abstracts consider itself initialized, as they have to be
		    // evaluated per case
		    true,
		    TypeKind::abstract, // abstract
		    // name cannot be mangled nor referenced
		    name,
		    // size is 0 so it cannot be passed
		    0,
		    false,     // non mutable
		    false,     // non signed
		    false,     // non overloaded
		    subType,   // subtype specified by caller
		    signature, // signature specified by caller
		};
	}

	// defines an empty tuple type, which holds different rules than a
	// conventional tuple (abstract vs aggregate)
	static constexpr Type defineUnitType(bool isMutable = false) {
		// TODO: revisit this section in the future to evaluate unit constness
		return {
		    // its type is unknown and can be changed later
		    0,
		    // initialized
		    true,
		    lang::TypeKind::abstract,
		    "%<tuple>%",
		    0,         // its size is 0
		    isMutable, // non mutable
		    false,     // non signed
		    false,     // cannot be overloaded
		    {},        // no subtype
		    {},        // no signature
		};
	}

  private:
	bool baseMatches(const Type &other) const;
};
} // namespace ray::compiler::lang
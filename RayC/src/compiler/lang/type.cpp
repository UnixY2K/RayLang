#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <optional>

#include <ray/compiler/lang/type.hpp>
namespace ray::compiler::lang {

bool Type::coercercesInto(const Type &targetType) const {
	// for now just validate wether the type is the same minus constness
	return baseMatches(
	           targetType) && // base has to match
	                          // base type either is mutable or matches
	                          // same level as target
	                          // for scalar types they can be trivially coerced
	                          // as its contents are copied directly
	       (isScalar() || isMutable || isMutable == targetType.isMutable) &&
	       signatureMatches(targetType); // signature is a heavier
	                                     // comparison that goes last
}

bool Type::signatureEquals(const Type &targetType) const {
	if (subtype.has_value() != targetType.subtype.has_value() ||
	    (subtype.has_value() &&
	     *subtype.value() != *targetType.subtype.value())) {
		return false;
	}
	if (signature.has_value()) {
		const auto &lhsSignature = signature.value();
		const auto &rhsSignature = targetType.signature.value();

		if (lhsSignature.size() != rhsSignature.size()) {
			return false;
		}
		for (size_t i = 0; i < lhsSignature.size(); i++) {
			if (*lhsSignature[i] != *rhsSignature[i]) {
				return false;
			}
		}
	}
	return true;
}

bool Type::signatureMatches(const Type &targetType) const {
	if (subtype.has_value() != targetType.subtype.has_value() ||
	    (subtype.has_value() &&
	     !subtype.value()->coercercesInto(*targetType.subtype.value()))) {
		return false;
	}
	if (signature.has_value()) {
		const auto &lhsSignature = signature.value();
		const auto &rhsSignature = targetType.signature.value();

		if (lhsSignature.size() != rhsSignature.size()) {
			return false;
		}
		for (size_t i = 0; i < lhsSignature.size(); i++) {
			if (lhsSignature[i]->coercercesInto(*rhsSignature[i].get())) {
				return false;
			}
		}
	}
	return true;
}

bool Type::operator==(const Type &other) const {
	return baseMatches(other) &&           // base has to match
	       isMutable == other.isMutable && // also constness as this is an
	                                       // stricter comparison
	       signatureEquals(
	           other); // signature is a heavier comparison that goes last
}

Type Type::defineStmtType() {
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

Type Type::defineModuleType() {
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

bool Type::baseMatches(const Type &other) const {
	return initialized == other.initialized &&       // has to be the same
	       scalar == other.scalar &&                 // |
	       name == other.name &&                     // |
	       calculatedSize == other.calculatedSize && // same size
	       isPointer == other.isPointer &&           // |
	       signedType == other.signedType;           // same signess
}

} // namespace ray::compiler::lang
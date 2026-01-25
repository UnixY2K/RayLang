#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {

size_t StructMember::calculateSize() const { return type.calculatedSize; }

} // namespace ray::compiler::lang

#include <cassert>
#include <optional>
#include <utility>

#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::lang {
std::optional<util::soft_reference<Struct>>
SourceUnit::declareStruct(const Struct &structObj) {
	auto val = this->structs.insert(std::make_pair(nextId, structObj));
	assert(val.second);
	auto &structRef = val.first->second;
	structRef.structID = nextId++;

	return {{structRef.structID, structRef}};
}
} // namespace ray::compiler::lang
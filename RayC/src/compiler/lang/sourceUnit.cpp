#include <cassert>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>

#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::lang {
bool SourceUnit::bindStruct(const Struct &structObj, Scope &scope) {
	auto val = this->structs.insert(std::make_pair(nextId, structObj));
	assert(val.second);
	auto &structRef = val.first->second;
	structRef.structID = nextId++;
	auto structSoftRef =
	    util::soft_reference<Struct>{structRef.structID, structRef};

	return scope.bindStruct(structObj.name, structSoftRef);
}

std::optional<std::reference_wrapper<Struct>>
SourceUnit::findStruct(std::string_view structName, Scope &currentScope) {

	return currentScope.findLocalStruct(structName)
	    .transform([](auto structRef) { return structRef.getObject().value(); })
	    .or_else([&] {
		    return currentScope.getParentScope().and_then(
		        [&](Scope &parentScope) {
			        return this->findStruct(structName, parentScope);
		        });
	    });
}
} // namespace ray::compiler::lang
#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::lang {

bool Scope::bindStruct(const std::string_view name,
                       util::soft_reference<Struct> &structRef) {
	auto val = structs.insert(std::make_pair(std::string(name), structRef));
	if (!val.second) {
		auto &sourceStructRef = val.first->second;
		if (sourceStructRef.getObjectId() != 0) {
			return false;
		}
		sourceStructRef = structRef;
	}
	return true;
}

bool Scope::declareStruct(const std::string_view name) {
	auto _ = structs.insert(
	    std::make_pair(std::string(name), util::soft_reference<Struct>()));
	return true;
}
bool Scope::defineFunction(FunctionDeclaration declaration) { return false; }
bool Scope::defineLocalVariable(const Symbol symbol) { return false; }

const std::optional<const util::soft_reference<Symbol>>
Scope::findVariable(const std::string_view name) const {
	return std::nullopt;
}

const std::optional<
    const std::vector<util::soft_reference<FunctionDeclaration>>>
Scope::findFunctionDeclaration(const std::string_view name) const {
	return std::nullopt;
}

const std::optional<const util::soft_reference<Struct>>
Scope::findStruct(const std::string_view name) const {
	return std::nullopt;
}

Scope &Scope::makeChildScope() {
	innerScopes.push_back(Scope(*this));
	return *innerScopes.back().get();
}

} // namespace ray::compiler::lang
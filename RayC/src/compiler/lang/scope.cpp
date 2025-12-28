#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

#include <ray/compiler/lang/scope.hpp>

namespace ray::compiler::lang {
bool Scope::declareStruct(Type type, const std::string_view mangledName) {
	return false;
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
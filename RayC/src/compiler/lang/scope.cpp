#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/scope.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/util/soft_reference.hpp>

namespace ray::compiler::lang {

bool Scope::bindStruct(Struct &&structObj) {
	assert(!structObj.opaque);
	assert(structs.contains(structObj.name));
	util::soft_reference<Struct> currentStructRef = structs.at(structObj.name);
	assert(currentStructRef.getObjectId() != 0);
	Struct &currentStructObj = currentStructRef.getObject().value().get();
	if (!currentStructObj.opaque) {
		return false;
	}

	currentStructObj = structObj;
	return true;
}
bool Scope::bindTrait(Trait &&traitObj) {
	assert(traits.contains(traitObj.name));
	util::soft_reference<Trait> currentTraitRef = traits.at(traitObj.name);
	assert(currentTraitRef.getObjectId() != 0);
	Trait &currentTraitObj = currentTraitRef.getObject().value().get();

	currentTraitObj = traitObj;
	return true;
}

bool Scope::bindFunctionDeclaration(
    std::string_view name,
    util::soft_reference<FunctionDeclaration> &functionDeclarationRef) {
	auto functionDeclaration = functionDeclarationRef.getObject()->get();
	if (!functions.contains(functionDeclaration.name)) {
		functions[functionDeclaration.name] =
		    std::vector<util::soft_reference<FunctionDeclaration>>{};
	}

	auto &declarations = functions.at(functionDeclaration.name);

	if (!declarations.empty() &&
	    declarations[0].getObject()->get().signature.returnType !=
	        functionDeclaration.signature.returnType) {
		// this should be probably an enum with an error code
		return false;
	}

	// search for any declaration that might have our same declaration
	// details
	for (const auto &declarationRef : declarations) {
		const auto &declaration = declarationRef.getObject()->get();
		bool matching = false;
		if (functionDeclaration.signature.parameters.size() ==
		    declaration.signature.parameters.size()) {
			for (size_t paramIndex = 0;
			     paramIndex < functionDeclaration.signature.parameters.size();
			     paramIndex++) {
				if (functionDeclaration.signature.parameters[paramIndex] !=
				    declaration.signature.parameters[paramIndex]) {
					matching = false;
					break;
				}
			}
		}
		if (matching) {
			return false;
		}
	}

	declarations.push_back(functionDeclarationRef);

	return true;
}

bool Scope::declareStruct(const util::soft_reference<Struct> &structRef) {
	assert(structRef.getObjectId() != 0);
	assert(structRef.getObject().has_value());
	auto structObj = structRef.getObject()->get();
	auto result = structs.insert(std::make_pair(structObj.name, structRef));
	return result.second;
}
bool Scope::declareTrait(const util::soft_reference<Trait> &traitRef) {
	assert(traitRef.getObjectId() != 0);
	assert(traitRef.getObject().has_value());
	auto traitObj = traitRef.getObject()->get();
	auto result = traits.insert(std::make_pair(traitObj.name, traitRef));
	return result.second;
}
bool Scope::declareLocalVariable(const util::soft_reference<Symbol> symbolRef) {
	assert(symbolRef.getObject().has_value());
	const auto &symbol = symbolRef.getObject().value().get();
	if (variables.contains(symbol.mangledName)) {
		return false;
	}
	variables[symbol.mangledName] = symbolRef;
	return true;
}

const std::optional<const util::soft_reference<Symbol>>
Scope::findVariable(const std::string_view name) const {
	const std::string key(name);
	if (variables.contains(key)) {
		return variables.at(key);
	}
	return std::nullopt;
}

const std::optional<
    const std::vector<util::soft_reference<FunctionDeclaration>>>
Scope::findLocalFunctionDeclaration(const std::string_view functionName) const {
	const std::string key(functionName);
	if (functions.contains(key)) {
		return functions.at(key);
	}
	return std::nullopt;
}

const std::optional<const util::soft_reference<Struct>>
Scope::findLocalStruct(const std::string_view name) const {
	std::string key = std::string(name);
	if (structs.contains(key)) {
		return structs.at(key);
	}
	return std::nullopt;
}

std::optional<util::soft_reference<Struct>>
Scope::findLocalStruct(const std::string_view name) {
	std::string key = std::string(name);
	if (structs.contains(key)) {
		return structs.at(key);
	}
	return std::nullopt;
}

const std::optional<const util::soft_reference<Trait>>
Scope::findLocalTrait(const std::string_view name) const {
	std::string key = std::string(name);
	if (traits.contains(key)) {
		return traits.at(key);
	}
	return std::nullopt;
}

std::optional<util::soft_reference<Trait>>
Scope::findLocalTrait(const std::string_view name) {
	std::string key = std::string(name);
	if (traits.contains(key)) {
		return traits.at(key);
	}
	return std::nullopt;
}

Scope &Scope::makeChildScope() {
	innerScopes.push_back(Scope(*this));
	return *innerScopes.back().get();
}

} // namespace ray::compiler::lang
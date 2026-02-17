#include "ray/compiler/lang/functionDefinition.hpp"
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
				     paramIndex <
				     functionDeclaration.signature.parameters.size();
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

bool Scope::declareStruct(const std::string_view name) {
	auto _ = structs.insert(
	    std::make_pair(std::string(name), util::soft_reference<Struct>()));
	return true;
}
bool Scope::declareLocalVariable(const Symbol symbol) { return false; }

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

const std::optional<const util::soft_reference<Struct>>
Scope::findStruct(const std::string_view name) const {
	return std::nullopt;
}

Scope &Scope::makeChildScope() {
	innerScopes.push_back(Scope(*this));
	return *innerScopes.back().get();
}

} // namespace ray::compiler::lang
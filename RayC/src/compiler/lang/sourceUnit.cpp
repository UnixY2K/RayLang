#include <cassert>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>

#include <ray/compiler/lang/functionDefinition.hpp>
#include <ray/compiler/lang/sourceUnit.hpp>
#include <ray/compiler/lang/struct.hpp>
#include <ray/compiler/lang/symbol.hpp>
#include <ray/util/soft_reference.hpp>
#include <vector>

namespace ray::compiler::lang {

bool SourceUnit::declareLocalVariable(const Symbol symbol, Scope &scope) {
	auto val = this->variables.insert(std::make_pair(nextId, symbol));
	assert(val.second);
	auto &variableRef = val.first->second;
	variableRef.symbolId = nextId++;
	auto variableSoftRef =
	    util::soft_reference<lang::Symbol>{variableRef.symbolId, variableRef};

	return scope.declareLocalVariable(variableSoftRef);
}
bool SourceUnit::declareFunction(const FunctionDeclaration &functionDeclaration,
                                 Scope &scope) {
	auto val =
	    this->functions.insert(std::make_pair(nextId, functionDeclaration));
	assert(val.second);
	auto &functionDeclarationRef = val.first->second;
	functionDeclarationRef.functionID = nextId++;
	auto functionDeclarationSoftRef =
	    util::soft_reference<lang::FunctionDeclaration>{
	        functionDeclarationRef.functionID, functionDeclarationRef};

	return scope.bindFunctionDeclaration(functionDeclaration.name,
	                                     functionDeclarationSoftRef);
}
bool SourceUnit::declareStruct(const Struct &structObj, Scope &scope) {
	assert(structObj.opaque);
	if (scope.findLocalStruct(structObj.name)) {
		return true;
	}

	auto val = this->structs.insert(std::make_pair(nextId, structObj));
	assert(val.second);
	auto &structRef = val.first->second;
	structRef.structID = nextId++;
	auto structSoftRef =
	    util::soft_reference<Struct>{structRef.structID, structRef};

	return scope.declareStruct(structSoftRef);
}
bool SourceUnit::declareTrait(const Trait &traitObj, Scope &scope) {

	if (scope.findLocalTrait(traitObj.name)) {
		return true;
	}

	auto val = this->traits.insert(std::make_pair(nextId, traitObj));
	assert(val.second);
	auto &traitRef = val.first->second;
	traitRef.traitID = nextId++;
	auto traitSoftRef = util::soft_reference<Trait>{traitRef.traitID, traitRef};

	return scope.declareTrait(traitSoftRef);
}

std::vector<util::soft_reference<FunctionDeclaration>>
SourceUnit::findFunctionDeclarations(const std::string_view functionName,
                                     const Scope &scope) const {
	std::vector<util::soft_reference<FunctionDeclaration>> functionDeclarations;

	for (auto *currentScope = &scope; currentScope;
	     currentScope =
	         currentScope->getParentScope()
	             .transform([](const std::reference_wrapper<Scope> &parentScope)
	                            -> const Scope * { return &parentScope.get(); })
	             .value_or(nullptr)) {

		auto localDeclarations =
		    currentScope->findLocalFunctionDeclaration(functionName)
		        .value_or(
		            std::vector<util::soft_reference<FunctionDeclaration>>());
		functionDeclarations.reserve(functionDeclarations.size() +
		                             localDeclarations.size());
		functionDeclarations.insert(functionDeclarations.end(),
		                            localDeclarations.begin(),
		                            localDeclarations.end());
	}

	return functionDeclarations;
}
std::optional<std::reference_wrapper<Struct>>
SourceUnit::findStruct(const std::string_view structName,
                       const Scope &currentScope) const {

	auto foundStruct = currentScope.findLocalStruct(structName);
	return foundStruct
	    .transform([](util::soft_reference<Struct> structRef) {
		    std::reference_wrapper<const Struct> valueRef =
		        structRef.getObject().value();
		    return structRef.getObject().value();
	    })
	    .or_else([&] {
		    return currentScope.getParentScope().and_then(
		        [&](Scope &parentScope) {
			        return this->findStruct(structName, parentScope);
		        });
	    });
}
} // namespace ray::compiler::lang
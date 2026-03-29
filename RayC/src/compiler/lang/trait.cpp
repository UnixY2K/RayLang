#include <ray/compiler/lang/trait.hpp>
#include <ray/compiler/lang/type.hpp>

namespace ray::compiler::lang {
Type MethodSignature::getMethodType(
    const environment::DataModel &dataModel) const {
	std::vector<util::copy_ptr<Type>> signature;
	for (const auto &parameter : parameters) {
		signature.push_back(parameter.parameterType);
	}
	return dataModel.defineMethodType(returnType, signature);
}

Type MethodSignature::getOverloadedMethodType(
    const environment::DataModel &dataModel) const {
	return dataModel.defineOverloadedMethodType(returnType);
}
} // namespace ray::compiler::lang
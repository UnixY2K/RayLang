#pragma once
#include <string>
#include <vector>

namespace ray::compiler::lang {
class SourceUnit {

  public:
	std::vector<std::string> requiredFiles;
};
} // namespace ray::compiler::lang
#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <ray/compiler/lexer/token.hpp>

namespace ray::compiler::infrastructure::diagnostics {

enum class DiagnosticSeverity {
	Warning, // warning level
	Error,   // error level
	Fatal,   // fatal level, evaluation cannot continue after this phase
	Bug      // ICE(internal compiler error)/compiler bugs
};

struct SourceLocation {
	std::size_t sourceId;
	size_t line{0};
	size_t column{0};
};

struct Diagnostic {
	DiagnosticSeverity severity;
	SourceLocation location;
	std::string category;
	std::string message;

	constexpr std::string_view severityAsString() const {
		return severityAsString(severity);
	}

	static constexpr std::string_view
	severityAsString(const DiagnosticSeverity severity) {
		switch (severity) {
		case DiagnosticSeverity::Warning:
			return "warning";
		case DiagnosticSeverity::Error:
			return "error";
		case DiagnosticSeverity::Fatal:
			return "fatal";
		case DiagnosticSeverity::Bug:
			return "bug";
		default:
			return "unknown";
		}
	};
};

class DiagnosticEngine {
  private:
	std::vector<Diagnostic> diagnostics;
	std::size_t currentSourceID{0};
	std::string currentPhase = "{UNASSIGNED}";

	bool failed = false;
	bool fatallyFailed = false;

  public:
	DiagnosticEngine() = default;

	// Contexto activo para el procesamiento de archivos
	void set_currentSourceID(const std::size_t sourceID) {
		currentSourceID = sourceID;
	}
	void set_category(std::string_view phaseName) { currentPhase = phaseName; }

	void error(const Token &token, std::string_view message);
	void warning(const Token &token, std::string_view message);
	void bug(const Token &token, std::string_view message);
	void fatal(const Token &token, std::string_view message);
	
	void error(size_t line, size_t column, std::string_view message);
	void warning(size_t line, size_t column, std::string_view message);
	void bug(size_t line, size_t column, std::string_view message);
	void fatal(size_t line, size_t column, std::string_view message);

	bool hasFailed() const { return failed; }
	bool hasFatallyFailed() const { return fatallyFailed; }

	const std::vector<Diagnostic> &getDiagnostics() const {
		return diagnostics;
	}

  private:
	void report(DiagnosticSeverity severity, SourceLocation location,
	            std::string_view message);
	static std::string escapeString(std::string_view string);
};

} // namespace ray::compiler::infrastructure::diagnostics

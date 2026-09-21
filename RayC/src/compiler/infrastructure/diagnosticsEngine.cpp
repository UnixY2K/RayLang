#include <sstream>

#include <ray/compiler/infrastructure/diagnosticsEngine.hpp>

namespace ray::compiler::infrastructure::diagnostics {

void DiagnosticEngine::error(const Token &token, std::string_view message) {
	error(token.line, token.column, message);
}
void DiagnosticEngine::warning(const Token &token, std::string_view message) {
	warning(token.line, token.column, message);
}
void DiagnosticEngine::bug(const Token &token, std::string_view message) {
	bug(token.line, token.column, message);
}
void DiagnosticEngine::fatal(const Token &token, std::string_view message) {
	fatal(token.line, token.column, message);
}

void DiagnosticEngine::error(size_t line, size_t column,
                             std::string_view message) {
	report(DiagnosticSeverity::Error,
	       SourceLocation{currentSourceID, line, column}, message);
}
void DiagnosticEngine::warning(size_t line, size_t column,
                               std::string_view message) {
	report(DiagnosticSeverity::Warning,
	       SourceLocation{currentSourceID, line, column}, message);
}
void DiagnosticEngine::bug(size_t line, size_t column,
                           std::string_view message) {
	report(DiagnosticSeverity::Bug,
	       SourceLocation{currentSourceID, line, column}, message);
}
void DiagnosticEngine::fatal(size_t line, size_t column,
                             std::string_view message) {
	report(DiagnosticSeverity::Fatal,
	       SourceLocation{currentSourceID, line, column}, message);
}

void DiagnosticEngine::report(DiagnosticSeverity severity,
                              SourceLocation location,
                              std::string_view message) {
	diagnostics.push_back(
	    Diagnostic{severity, location, currentPhase, std::string(message)});
}
std::string DiagnosticEngine::escapeString(std::string_view string) {
	std::stringstream output;
	for (const char c : string) {
		switch (c) {
		case '\a':
			output << "\\a";
			break;
		case '\b':
			output << "\\b";
			break;
		case '\e':
			output << "\\e";
			break;
		case '\f':
			output << "\\f";
			break;
		case '\n':
			output << "\\n";
			break;
		case '\r':
			output << "\\r";
			break;
		case '\v':
			output << "\\v";
			break;
		case '\'':
			output << "'";
			break;
		case '"':
			output << '"';
			break;
		case '?':
			output << '?';
			break;
		default:
			output << c;
		}
	}
	return output.str();
}
} // namespace ray::compiler::infrastructure::diagnostics
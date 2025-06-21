#include <ray/compiler/lexer/token.hpp>

#include <format>
#include <string_view>
#include <unordered_map>

namespace ray::compiler {
std::string Token::toString() const {
	return std::format(
	    "Token{{type: {:<25},line: {:<4},char: {:<4},lexeme: '{}'}}",
	    toString(type), line, column, getLexeme());
}

std::string_view Token::getLexeme() const {
	return lexeme.empty() ? glyph(type) : lexeme;
}
std::string_view Token::getGlyph() const { return glyph(type); }

void Token::merge(Token other) { lexeme += other.lexeme; }

Token::TokenType Token::fromChar(const char c) { return fromString({&c, 1}); }
Token::TokenType Token::fromString(std::string_view str) {
	static std::unordered_map<std::string, TokenType> map = {
	    // block tokens
	    {"(", Token::TokenType::TOKEN_LEFT_PAREN},         // (
	    {")", Token::TokenType::TOKEN_RIGHT_PAREN},        // )
	    {"{", Token::TokenType::TOKEN_LEFT_BRACE},         // {
	    {"}", Token::TokenType::TOKEN_RIGHT_BRACE},        // }
	    {"[", Token::TokenType::TOKEN_LEFT_SQUARE_BRACE},  // [
	    {"]", Token::TokenType::TOKEN_RIGHT_SQUARE_BRACE}, // ]
	    // assignment
	    {"=", Token::TokenType::TOKEN_EQUAL},               // =
	    {"+=", Token::TokenType::TOKEN_PLUS_EQUAL},         // +=
	    {"-=", Token::TokenType::TOKEN_MINUS_EQUAL},        // -=
	    {"*=", Token::TokenType::TOKEN_STAR_EQUAL},         // *=
	    {"/=", Token::TokenType::TOKEN_SLASH_EQUAL},        // /=
	    {"*=", Token::TokenType::TOKEN_PERCENT_EQUAL},      // *=
	    {"&=", Token::TokenType::TOKEN_AMPERSAND_EQUAL},    // &=
	    {"|=", Token::TokenType::TOKEN_PIPE_EQUAL},         // |=
	    {"^=", Token::TokenType::TOKEN_CARET_EQUAL},        // ^=
	    {"<<=", Token::TokenType::TOKEN_LESS_LESS_EQUAL},   // <<=
	    {">>=", Token::TokenType::TOKEN_GREAT_GREAT_EQUAL}, // >>=
	    // increment, decrement
	    {"++", Token::TokenType::TOKEN_PLUS_PLUS},   // ++
	    {"--", Token::TokenType::TOKEN_MINUS_MINUS}, // --
	    // arithmetic
	    {"+", Token::TokenType::TOKEN_PLUS},         // +
	    {"-", Token::TokenType::TOKEN_MINUS},        // -
	    {"*", Token::TokenType::TOKEN_STAR},         // *
	    {"/", Token::TokenType::TOKEN_SLASH},        // /
	    {"%", Token::TokenType::TOKEN_PERCENT},      // %
	    {"&", Token::TokenType::TOKEN_AMPERSAND},    // &
	    {"|", Token::TokenType::TOKEN_PIPE},         // |
	    {"^", Token::TokenType::TOKEN_CARET},        // ^
	    {"<<", Token::TokenType::TOKEN_LESS_LESS},   // <<
	    {">>", Token::TokenType::TOKEN_GREAT_GREAT}, // >>
	    // logical
	    {"!", Token::TokenType::TOKEN_BANG},                 // !
	    {"&&", Token::TokenType::TOKEN_AMPERSAND_AMPERSAND}, // &&
	    {"||", Token::TokenType::TOKEN_PIPE_PIPE},           // ||
	    // comparison
	    {"==", Token::TokenType::TOKEN_EQUAL_EQUAL}, // ==
	    {"!=", Token::TokenType::TOKEN_BANG_EQUAL},  // !=
	    {"<", Token::TokenType::TOKEN_LESS},         // <
	    {">", Token::TokenType::TOKEN_GREAT},        // >
	    {"<=", Token::TokenType::TOKEN_LESS_EQUAL},  // <=
	    {">=", Token::TokenType::TOKEN_GREAT_EQUAL}, // >=
	    // misc
	    {".", Token::TokenType::TOKEN_DOT},       // .
	    {",", Token::TokenType::TOKEN_COMMA},     // ,
	    {"?", Token::TokenType::TOKEN_QUESTION},  // ?
	    {":", Token::TokenType::TOKEN_COLON},     // :
	    {";", Token::TokenType::TOKEN_SEMICOLON}, // ;
	    {"->", Token::TokenType::TOKEN_ARROW},    // ->
	    {"#", Token::TokenType::TOKEN_POUND},     // #
	    // literals
	    //{"", Token::TokenType::TOKEN_IDENTIFIER}, // ex: foo, bar, baz, etc.
	    //{"", Token::TokenType::TOKEN_STRING},     // ex: "Hello, world"
	    //{"", Token::TokenType::TOKEN_NUMBER},     // ex: 0x01, 10, 10.2, -2,
	    //.1
	    //{"", Token::TokenType::TOKEN_CHAR},     	// ex: 'A', 'B'
	    //{"", Token::TokenType::TOKEN_INTRINSIC},  // ex: @sizeOf
	    // keywords
	    {"if", Token::TokenType::TOKEN_IF},               // if
	    {"else", Token::TokenType::TOKEN_ELSE},           // else
	    {"true", Token::TokenType::TOKEN_TRUE},           // true
	    {"false", Token::TokenType::TOKEN_FALSE},         // false
	    {"for", Token::TokenType::TOKEN_FOR},             // for
	    {"while", Token::TokenType::TOKEN_WHILE},         // while
	    {"fn", Token::TokenType::TOKEN_FN},               // fn
	    {"let", Token::TokenType::TOKEN_LET},             // let
	    {"return", Token::TokenType::TOKEN_RETURN},       // return
	    {"continue", Token::TokenType::TOKEN_CONTINUE},   // continue
	    {"break", Token::TokenType::TOKEN_BREAK},         // break
	    {"pub", Token::TokenType::TOKEN_PUB},             // pub
	    {"mut", Token::TokenType::TOKEN_MUT},             // mut
	    {"struct", Token::TokenType::TOKEN_STRUCT},       // struct
	    {"as", Token::TokenType::TOKEN_AS},               // as
	    {"namespace", Token::TokenType::TOKEN_NAMESPACE}, // namespace
	    {"import", Token::TokenType::TOKEN_IMPORT},       // import
	    // Token Types
	    {"()", Token::TokenType::TOKEN_TYPE_UNIT}, // ()
	};
	std::string key{str};
	return map.contains(key) ? map.at(key) : TokenType::TOKEN_ERROR;
}

std::string_view Token::toString(TokenType token) {
	switch (token) {
	// NON Used, only to hint that the token is not initialized
	case TokenType::TOKEN_UNINITIALIZED:
		return "TOKEN_UNINITIALIZED";
	// block tokens
	case TokenType::TOKEN_LEFT_PAREN:
		return "TOKEN_LEFT_PAREN";
	case TokenType::TOKEN_RIGHT_PAREN:
		return "TOKEN_RIGHT_PAREN";
	case TokenType::TOKEN_LEFT_BRACE:
		return "TOKEN_LEFT_BRACE";
	case TokenType::TOKEN_RIGHT_BRACE:
		return "TOKEN_RIGHT_BRACE";
	case TokenType::TOKEN_LEFT_SQUARE_BRACE:
		return "TOKEN_LEFT_SQUARE_BRACE";
	case TokenType::TOKEN_RIGHT_SQUARE_BRACE:
		return "TOKEN_RIGHT_SQUARE_BRACE";
	// assignment
	case TokenType::TOKEN_EQUAL:
		return "TOKEN_EQUAL";
	case TokenType::TOKEN_PLUS_EQUAL:
		return "TOKEN_PLUS_EQUAL";
	case TokenType::TOKEN_MINUS_EQUAL:
		return "TOKEN_MINUS_EQUAL";
	case TokenType::TOKEN_STAR_EQUAL:
		return "TOKEN_STAR_EQUAL";
	case TokenType::TOKEN_SLASH_EQUAL:
		return "TOKEN_SLASH_EQUAL";
	case TokenType::TOKEN_PERCENT_EQUAL:
		return "TOKEN_PERCENT_EQUAL";
	case TokenType::TOKEN_AMPERSAND_EQUAL:
		return "TOKEN_AMPERSAND_EQUAL";
	case TokenType::TOKEN_PIPE_EQUAL:
		return "TOKEN_PIPE_EQUAL";
	case TokenType::TOKEN_CARET_EQUAL:
		return "TOKEN_CARET_EQUAL";
	case TokenType::TOKEN_LESS_LESS_EQUAL:
		return "TOKEN_LESS_LESS_EQUAL";
	case TokenType::TOKEN_GREAT_GREAT_EQUAL:
		return "TOKEN_GREAT_GREAT_EQUAL";
	// increment, decrement
	case TokenType::TOKEN_PLUS_PLUS:
		return "TOKEN_PLUS_PLUS";
	case TokenType::TOKEN_MINUS_MINUS:
		return "TOKEN_MINUS_MINUS";
	// arithmetic
	case TokenType::TOKEN_PLUS:
		return "TOKEN_PLUS";
	case TokenType::TOKEN_MINUS:
		return "TOKEN_MINUS";
	case TokenType::TOKEN_STAR:
		return "TOKEN_STAR";
	case TokenType::TOKEN_SLASH:
		return "TOKEN_SLASH";
	case TokenType::TOKEN_PERCENT:
		return "TOKEN_PERCENT";
	case TokenType::TOKEN_AMPERSAND:
		return "TOKEN_AMPERSAND";
	case TokenType::TOKEN_PIPE:
		return "TOKEN_PIPE";
	case TokenType::TOKEN_CARET:
		return "TOKEN_CARET";
	case TokenType::TOKEN_LESS_LESS:
		return "TOKEN_LESS_LESS";
	case TokenType::TOKEN_GREAT_GREAT:
		return "TOKEN_GREAT_GREAT";
	// logical
	case TokenType::TOKEN_BANG:
		return "TOKEN_BANG";
	case TokenType::TOKEN_AMPERSAND_AMPERSAND:
		return "TOKEN_AMPERSAND_AMPERSAND";
	case TokenType::TOKEN_PIPE_PIPE:
		return "TOKEN_PIPE_PIPE";
	// comparison
	case TokenType::TOKEN_EQUAL_EQUAL:
		return "TOKEN_EQUAL_EQUAL";
	case TokenType::TOKEN_BANG_EQUAL:
		return "TOKEN_BANG_EQUAL";
	case TokenType::TOKEN_LESS:
		return "TOKEN_LESS";
	case TokenType::TOKEN_GREAT:
		return "TOKEN_GREAT";
	case TokenType::TOKEN_LESS_EQUAL:
		return "TOKEN_LESS_EQUAL";
	case TokenType::TOKEN_GREAT_EQUAL:
		return "TOKEN_GREAT_EQUAL";
	// MISC
	case TokenType::TOKEN_DOT:
		return "TOKEN_DOT";
	case TokenType::TOKEN_COMMA:
		return "TOKEN_COMMA";
	case TokenType::TOKEN_QUESTION:
		return "TOKEN_QUESTION";
	case TokenType::TOKEN_COLON:
		return "TOKEN_COLON";
	case TokenType::TOKEN_SEMICOLON:
		return "TOKEN_SEMICOLON";
	case TokenType::TOKEN_ARROW:
		return "TOKEN_ARROW";
	case TokenType::TOKEN_POUND:
		return "TOKEN_POUND";
	// Literals.
	case TokenType::TOKEN_IDENTIFIER:
		return "TOKEN_IDENTIFIER";
	case TokenType::TOKEN_STRING:
		return "TOKEN_STRING";
	case TokenType::TOKEN_NUMBER:
		return "TOKEN_NUMBER";
	case TokenType::TOKEN_CHAR:
		return "TOKEN_CHAR";
	case TokenType::TOKEN_INTRINSIC:
		return "TOKEN_INTRINSIC";
	// keywords
	case TokenType::TOKEN_IF:
		return "TOKEN_IF";
	case TokenType::TOKEN_ELSE:
		return "TOKEN_ELSE";
	case TokenType::TOKEN_TRUE:
		return "TOKEN_TRUE";
	case TokenType::TOKEN_FALSE:
		return "TOKEN_FALSE";
	case TokenType::TOKEN_FOR:
		return "TOKEN_FOR";
	case TokenType::TOKEN_WHILE:
		return "TOKEN_WHILE";
	case TokenType::TOKEN_FN:
		return "TOKEN_FN";
	case TokenType::TOKEN_LET:
		return "TOKEN_LET";
	case TokenType::TOKEN_RETURN:
		return "TOKEN_RETURN";
	case TokenType::TOKEN_CONTINUE:
		return "TOKEN_CONTINUE";
	case TokenType::TOKEN_BREAK:
		return "TOKEN_BREAK";
	case TokenType::TOKEN_PUB:
		return "TOKEN_PUB";
	case TokenType::TOKEN_MUT:
		return "TOKEN_MUT";
	case TokenType::TOKEN_STRUCT:
		return "TOKEN_STRUCT";
	case TokenType::TOKEN_AS:
		return "TOKEN_AS";
	case TokenType::TOKEN_NAMESPACE:
		return "TOKEN_NAMESPACE";
	case TokenType::TOKEN_IMPORT:
		return "TOKEN_IMPORT";
	// token types
	case TokenType::TOKEN_TYPE_UNIT:
		return "TOKEN_TYPE_UNIT";
	// other
	case TokenType::TOKEN_ERROR:
		return "TOKEN_ERROR";
	case TokenType::TOKEN_EOF:
		return "TOKEN_EOF";
	}
	return "???";
}
std::string_view Token::glyph(TokenType token) {
	switch (token) {
	// NON Used, only to hint that the token is not initialized
	case TokenType::TOKEN_UNINITIALIZED:
		return "TOKEN_UNINITIALIZED";
	// block tokens
	case TokenType::TOKEN_LEFT_PAREN:
		return "(";
	case TokenType::TOKEN_RIGHT_PAREN:
		return ")";
	case TokenType::TOKEN_LEFT_BRACE:
		return "{";
	case TokenType::TOKEN_RIGHT_BRACE:
		return "}";
	case TokenType::TOKEN_LEFT_SQUARE_BRACE:
		return "[";
	case TokenType::TOKEN_RIGHT_SQUARE_BRACE:
		return "]";
	// assignment
	case TokenType::TOKEN_EQUAL:
		return "=";
	case TokenType::TOKEN_PLUS_EQUAL:
		return "+=";
	case TokenType::TOKEN_MINUS_EQUAL:
		return "-=";
	case TokenType::TOKEN_STAR_EQUAL:
		return "*=";
	case TokenType::TOKEN_SLASH_EQUAL:
		return "/=";
	case TokenType::TOKEN_PERCENT_EQUAL:
		return "%=";
	case TokenType::TOKEN_AMPERSAND_EQUAL:
		return "&=";
	case TokenType::TOKEN_PIPE_EQUAL:
		return "|=";
	case TokenType::TOKEN_CARET_EQUAL:
		return "^=";
	case TokenType::TOKEN_LESS_LESS_EQUAL:
		return "<<=";
	case TokenType::TOKEN_GREAT_GREAT_EQUAL:
		return ">>=";
	// increment, decrement
	case TokenType::TOKEN_PLUS_PLUS:
		return "++";
	case TokenType::TOKEN_MINUS_MINUS:
		return "--";
	// arithmetic
	case TokenType::TOKEN_PLUS:
		return "+";
	case TokenType::TOKEN_MINUS:
		return "-";
	case TokenType::TOKEN_STAR:
		return "*";
	case TokenType::TOKEN_SLASH:
		return "/";
	case TokenType::TOKEN_PERCENT:
		return "%";
	case TokenType::TOKEN_AMPERSAND:
		return "&";
	case TokenType::TOKEN_PIPE:
		return "|";
	case TokenType::TOKEN_CARET:
		return "^";
	case TokenType::TOKEN_LESS_LESS:
		return "<<";
	case TokenType::TOKEN_GREAT_GREAT:
		return ">>";
	// logical
	case TokenType::TOKEN_BANG:
		return "!";
	case TokenType::TOKEN_AMPERSAND_AMPERSAND:
		return "&&";
	case TokenType::TOKEN_PIPE_PIPE:
		return "||";
	// comparison
	case TokenType::TOKEN_EQUAL_EQUAL:
		return "==";
	case TokenType::TOKEN_BANG_EQUAL:
		return "!=";
	case TokenType::TOKEN_LESS:
		return "<";
	case TokenType::TOKEN_GREAT:
		return ">";
	case TokenType::TOKEN_LESS_EQUAL:
		return "<=";
	case TokenType::TOKEN_GREAT_EQUAL:
		return ">=";
	// MISC
	case TokenType::TOKEN_DOT:
		return ".";
	case TokenType::TOKEN_COMMA:
		return ",";
	case TokenType::TOKEN_QUESTION:
		return "?";
	case TokenType::TOKEN_COLON:
		return ":";
	case TokenType::TOKEN_SEMICOLON:
		return ";";
	case TokenType::TOKEN_ARROW:
		return "->";
	case TokenType::TOKEN_POUND:
		return "#";
	// Literals.
	case TokenType::TOKEN_IDENTIFIER:
		return "<identifier>";
	case TokenType::TOKEN_STRING:
		return "<string>";
	case TokenType::TOKEN_NUMBER:
		return "<number>";
	case TokenType::TOKEN_CHAR:
		return "<char-literal>";
	case TokenType::TOKEN_INTRINSIC:
		return "<intrinsic>";
	// keywords
	case TokenType::TOKEN_IF:
		return "if";
	case TokenType::TOKEN_ELSE:
		return "else";
	case TokenType::TOKEN_TRUE:
		return "true";
	case TokenType::TOKEN_FALSE:
		return "false";
	case TokenType::TOKEN_FOR:
		return "for";
	case TokenType::TOKEN_WHILE:
		return "while";
	case TokenType::TOKEN_FN:
		return "fn";
	case TokenType::TOKEN_LET:
		return "let";
	case TokenType::TOKEN_RETURN:
		return "return";
	case TokenType::TOKEN_CONTINUE:
		return "continue";
	case TokenType::TOKEN_BREAK:
		return "break";
	case TokenType::TOKEN_PUB:
		return "pub";
	case TokenType::TOKEN_MUT:
		return "mut";
	case TokenType::TOKEN_STRUCT:
		return "struct";
	case TokenType::TOKEN_AS:
		return "as";
	case TokenType::TOKEN_NAMESPACE:
		return "namespace";
	case TokenType::TOKEN_IMPORT:
		return "import";
	// token types
	case TokenType::TOKEN_TYPE_UNIT:
		return "()";
	// other
	case TokenType::TOKEN_ERROR:
		return "<{TOKEN_ERROR}>";
	case TokenType::TOKEN_EOF:
		return "<{EOF}>";
	}
	return "<{???}>";
}

namespace types {
Token makeUnitTypeToken(size_t line, size_t column) {
	auto val = Token::fromString("()");
	return Token{val, "()", line, column};
}
} // namespace types

} // namespace ray::compiler

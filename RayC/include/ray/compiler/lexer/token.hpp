#pragma once
#include <cstddef>
#include <string>
#include <string_view>

namespace ray::compiler {

struct Token {
	enum class TokenType {
		// NON Used, only to hint that the token is not initialized
		TOKEN_UNINITIALIZED,
		// block tokens
		TOKEN_LEFT_PAREN,         // (
		TOKEN_RIGHT_PAREN,        // )
		TOKEN_LEFT_BRACE,         // {
		TOKEN_RIGHT_BRACE,        // }
		TOKEN_LEFT_SQUARE_BRACE,  // [
		TOKEN_RIGHT_SQUARE_BRACE, // ]
		// assignment
		TOKEN_EQUAL,             // =
		TOKEN_PLUS_EQUAL,        // +=
		TOKEN_MINUS_EQUAL,       // -=
		TOKEN_STAR_EQUAL,        // *=
		TOKEN_SLASH_EQUAL,       // /=
		TOKEN_PERCENT_EQUAL,     // *=
		TOKEN_AMPERSAND_EQUAL,   // &=
		TOKEN_PIPE_EQUAL,        // |=
		TOKEN_CARET_EQUAL,       // ^=
		TOKEN_LESS_LESS_EQUAL,   // <<=
		TOKEN_GREAT_GREAT_EQUAL, // >>=
		// increment, decrement
		TOKEN_PLUS_PLUS,   // ++
		TOKEN_MINUS_MINUS, // --
		// arithmetic
		TOKEN_PLUS,        // +
		TOKEN_MINUS,       // -
		TOKEN_STAR,        // *
		TOKEN_SLASH,       // /
		TOKEN_PERCENT,     // %
		TOKEN_AMPERSAND,   // &
		TOKEN_PIPE,        // |
		TOKEN_CARET,       // ^
		TOKEN_LESS_LESS,   // <<
		TOKEN_GREAT_GREAT, // >>
		// logical
		TOKEN_BANG,                // !
		TOKEN_AMPERSAND_AMPERSAND, // &&
		TOKEN_PIPE_PIPE,           // ||
		// comparison
		TOKEN_EQUAL_EQUAL, // ==
		TOKEN_BANG_EQUAL,  // !=
		TOKEN_LESS,        // <
		TOKEN_GREAT,       // >
		TOKEN_LESS_EQUAL,  // <=
		TOKEN_GREAT_EQUAL, // >=
		// MISC
		TOKEN_DOT,       // .
		TOKEN_COMMA,     // ,
		TOKEN_QUESTION,  // ?
		TOKEN_COLON,     // :
		TOKEN_SEMICOLON, // ;
		TOKEN_ARROW,     // ->
		TOKEN_POUND,     // #
		// Literals.
		TOKEN_IDENTIFIER, // ex: foo, bar, baz, etc.
		TOKEN_STRING,     // ex: "Hello, world"
		TOKEN_NUMBER,     // ex: 0x01, 10, 10.2, -2
		TOKEN_CHAR,       // ex: 'a', 'b', 'c'
		TOKEN_INTRINSIC,  // ex: @sizeOf
		// Keywords.
		TOKEN_IF,        // if
		TOKEN_ELSE,      // else
		TOKEN_TRUE,      // true
		TOKEN_FALSE,     // false
		TOKEN_FOR,       // for
		TOKEN_WHILE,     // while
		TOKEN_FN,        // fn
		TOKEN_LET,       // let
		TOKEN_RETURN,    // return
		TOKEN_CONTINUE,  // continue
		TOKEN_BREAK,     // break
		TOKEN_PUB,       // pub
		TOKEN_MUT,       // mut
		TOKEN_STRUCT,    // struct
		TOKEN_AS,        // as, ex: 2 as isize
		TOKEN_IMPORT,    // import ex: import "hello.ray"
		// token types
		TOKEN_TYPE_UNIT, // unit aka '()'
		// other
		TOKEN_ERROR, // when a token failed to parse
		TOKEN_EOF    // EOF
	};

	TokenType type;
	std::string lexeme;
	size_t line;
	size_t column;

	std::string toString() const;
	std::string_view getLexeme() const;
	std::string_view getGlyph() const;

	void merge(Token other);

	static TokenType fromChar(const char c);
	static TokenType fromString(std::string_view str);
	static std::string_view toString(TokenType token);
	static std::string_view glyph(TokenType token);
};

namespace types {
Token makeUnitTypeToken(size_t line, size_t column);
}

} // namespace ray::compiler

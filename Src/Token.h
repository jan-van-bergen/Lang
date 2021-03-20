#pragma once

#include <stdbool.h>

typedef enum Token_Type {
	TOKEN_IDENTIFIER,
	TOKEN_LITERAL_INT,
	TOKEN_LITERAL_F32,
	TOKEN_LITERAL_F64,
	TOKEN_LITERAL_BOOL,
	TOKEN_LITERAL_CHAR,
	TOKEN_LITERAL_STRING,

	TOKEN_DOT,

	TOKEN_KEYWORD_LET,      // let
	TOKEN_KEYWORD_CAST,     // cast()
	TOKEN_KEYWORD_SIZEOF,   // sizeof
	TOKEN_KEYWORD_EXTERN,   // extern
	TOKEN_KEYWORD_IF,       // if
	TOKEN_KEYWORD_ELSE,     // else
	//TOKEN_KEYWORD_FOR,	// for
	TOKEN_KEYWORD_WHILE,    // while
	TOKEN_KEYWORD_BREAK,    // break
	TOKEN_KEYWORD_CONTINUE, // continue
	TOKEN_KEYWORD_FUNC,     // func
	TOKEN_KEYWORD_RETURN,   // return
	TOKEN_KEYWORD_STRUCT,   // struct
		
	TOKEN_PARENTESES_OPEN,  // (
	TOKEN_PARENTESES_CLOSE, // )

	TOKEN_BRACES_OPEN,  // {
	TOKEN_BRACES_CLOSE, // }

	TOKEN_SQUARE_BRACES_OPEN,  // [
	TOKEN_SQUARE_BRACES_CLOSE, // ]

	TOKEN_ARROW, // ->
	
	TOKEN_ASSIGN,          // =
	TOKEN_ASSIGN_PLUS,     // +=
	TOKEN_ASSIGN_MINUS,    // -=
	TOKEN_ASSIGN_MULTIPLY, // *=
	TOKEN_ASSIGN_DIVIDE,   // /=
	//TOKEN_ASSIGN_MODULO,   // %=

	TOKEN_OPERATOR_PLUS,	 // +
	TOKEN_OPERATOR_MINUS,	 // -
	TOKEN_OPERATOR_MULTIPLY, // *
	TOKEN_OPERATOR_DIVIDE,	 // /
	TOKEN_OPERATOR_MODULO,	 // %

	TOKEN_OPERATOR_LT_EQ, // <=
	TOKEN_OPERATOR_GT_EQ, // >= 
	TOKEN_OPERATOR_LT,    // <
	TOKEN_OPERATOR_GT,    // >

	TOKEN_OPERATOR_EQ, // ==
	TOKEN_OPERATOR_NE, // !=

	TOKEN_OPERATOR_SHIFT_LEFT,  // <<
	TOKEN_OPERATOR_SHIFT_RIGHT, // >>

	TOKEN_OPERATOR_LOGICAL_AND, // &&
	TOKEN_OPERATOR_LOGICAL_OR,  // ||
	TOKEN_OPERATOR_LOGICAL_NOT, // !

	TOKEN_OPERATOR_BITWISE_AND, // &
	TOKEN_OPERATOR_BITWISE_OR,  // |
	TOKEN_OPERATOR_BITWISE_XOR, // ^
	TOKEN_OPERATOR_BITWISE_NOT, // ~

	TOKEN_OPERATOR_INC,  // ++
	TOKEN_OPERATOR_DEC,  // --

	TOKEN_COMMA,     // ,
	TOKEN_COLON,     // :
	TOKEN_SEMICOLON, // ;

	TOKEN_EOF
} Token_Type;

typedef struct Token {
	Token_Type type;

	int line;

	union {
		struct {
			unsigned long long value_int;
			bool sign;
		};

		float  value_float;
		double value_double;

		char const * value_str;
	};
} Token;

void token_to_string(Token const * token, char * string, int string_size);

#pragma once

typedef enum Token_Type {
	TOKEN_IDENTIFIER,
	TOKEN_LITERAL_STRING,
	TOKEN_LITERAL_INT,
		
	TOKEN_PARENTESES_OPEN,
	TOKEN_PARENTESES_CLOSE,

	TOKEN_COMMA,
	TOKEN_COLON,
	TOKEN_EQUALS,
	TOKEN_SEMICOLON
} Token_Type;

typedef struct Token {
	Token_Type type;

	union {
		char const * value_str;
		char         value_char;
		int          value_int;
	};
} Token;

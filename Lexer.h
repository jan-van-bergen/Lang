#pragma once
#include <stdbool.h>

typedef enum Token_Type {
	TOKEN_IDENTIFIER,
	TOKEN_LITERAL_INT,
	TOKEN_LITERAL_STRING,

	TOKEN_KEYWORD_IF,	  // if
	TOKEN_KEYWORD_ELSE,	  // else
	TOKEN_KEYWORD_FOR,	  // for
	TOKEN_KEYWORD_WHILE,  // while
	TOKEN_KEYWORD_STRUCT, // struct
		
	TOKEN_PARENTESES_OPEN,  // (
	TOKEN_PARENTESES_CLOSE, // )

	TOKEN_BRACES_OPEN,  // {
	TOKEN_BRACES_CLOSE, // }

	TOKEN_OPERATOR_PLUS,	 // +
	TOKEN_OPERATOR_MINUS,	 // -
	TOKEN_OPERATOR_MULTIPLY, // *
	TOKEN_OPERATOR_DIVIDE,	 // /
	//TOKEN_OPERATOR_MODULO,	 // %

	TOKEN_OPERATOR_EQUALS,     // ==
	TOKEN_OPERATOR_NOT_EQUALS, // !=

	//TOKEN_OPERATOR_LOGICAL_AND, // &&
	//TOKEN_OPERATOR_LOGICAL_OR,  // ||
	//TOKEN_OPERATOR_LOGICAL_NOT, // !

	//TOKEN_OPERATOR_BITWISE_AND, // &
	//TOKEN_OPERATOR_BITWISE_OR,  // |
	//TOKEN_OPERATOR_BITWISE_NOT, // ~

	TOKEN_ASSIGN, // =

	TOKEN_COMMA,     // ,
	TOKEN_COLON,     // :
	TOKEN_SEMICOLON, // ;
} Token_Type;

typedef struct Token {
	Token_Type type;

	union {
		char const * value_str;
		char         value_char;
		int          value_int;
	};
} Token;

void token_to_string(Token const * token, char * string, int string_size);

typedef struct Lexer {
	int          source_len;
	char const * source;

	int index;
} Lexer;

void lexer_init(Lexer * lexer, char const * source);

bool lexer_reached_end(Lexer const * lexer);

void lexer_next_token(Lexer * lexer, Token * token);

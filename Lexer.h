#pragma once
#include "Token.h"

typedef struct Lexer {
	const char * source;

	char curr;
	int index;
} Lexer;

void lexer_init(Lexer * lexer, const char * source);

void lexer_next(Lexer * lexer);

void lexer_skip(Lexer * lexer);

void lexer_next_token(Lexer * lexer, Token * token);

void lexer_get_string(Lexer * lexer);

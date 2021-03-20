#pragma once
#include <stdbool.h>

#include "Token.h"

typedef struct Lexer {
	int          source_len;
	char const * source;

	int index;
	int line;

	int     tokens_len;
	int     tokens_cap;
	Token * tokens;
} Lexer;

void lexer_init(Lexer * lexer, char const * source);
void lexer_free(Lexer * lexer);

void lexer_lex(Lexer * lexer);

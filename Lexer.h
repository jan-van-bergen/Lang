#pragma once
#include <stdbool.h>

#include "Token.h"

typedef struct Lexer {
	int          source_len;
	char const * source;

	int index;
} Lexer;

void lexer_init(Lexer * lexer, char const * source);

bool lexer_reached_end(Lexer const * lexer);

void lexer_get_token(Lexer * lexer, Token * token);

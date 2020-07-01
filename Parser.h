#pragma once
#include "AST.h"

#include <stdbool.h>

typedef struct Parser {
	int           token_count;
	Token const * tokens;

	int index;

	Variable_Buffer * current_variable_buffer;
	Scope           * current_scope;
} Parser;

void parser_init(Parser * parser, Token const * tokens, int token_count);

AST_Statement * parser_parse_program(Parser * parser);

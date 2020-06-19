#pragma once
#include "AST.h"

typedef struct Parser {
	int           token_count;
	Token const * tokens;

	int index;
} Parser;

void parser_init(Parser * parser, Token const * tokens, int token_count);

void parse_program(Parser * parser, AST_Statement * stat);

void parse_statement(Parser * parser, AST_Statement * stat);

void parse_expression(Parser * parser, AST_Expression * expr);

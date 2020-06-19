#pragma once
#include "AST.h"

#include <stdbool.h>

typedef struct Parser {
	int           token_count;
	Token const * tokens;

	int index;
} Parser;

void parser_init(Parser * parser, Token const * tokens, int token_count);

// Program
bool parser_match_program(Parser * parser);

// Statements
bool parser_match_statements(Parser * parser);

bool parser_match_statement(Parser * parser);

bool parser_match_statement_decl(Parser * parser);

bool parser_match_statement_assign(Parser * parser);

bool parser_match_statement_if(Parser * parser);

bool parser_match_statement_block(Parser * parser);

// Expressions
bool parser_match_expression_relational(Parser * parser);

bool parser_match_expression_arithmetic(Parser * parser);

bool parser_match_expression_term(Parser * parser);

bool parser_match_expression_factor(Parser * parser);

// Program
AST_Node * parser_parse_program(Parser * parser);

// Statements
AST_Node * parser_parse_statements(Parser * parser);

AST_Node * parser_parse_statement(Parser * parser);

AST_Node * parser_parse_statement_decl(Parser * parser);

AST_Node * parser_parse_statement_assign(Parser * parser);

AST_Node * parser_parse_statement_if(Parser * parser);

AST_Node * parser_parse_statement_block(Parser * parser);

// Expression
AST_Node * parser_parse_expression_relational(Parser * parser);

AST_Node * parser_parse_expression_arithmetic(Parser * parser);

AST_Node * parser_parse_expression_term(Parser * parser);

AST_Node * parser_parse_expression_factor(Parser * parser);

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
bool parser_match_statement(Parser * parser);

bool parser_match_statement_expr(Parser * parser);

bool parser_match_statement_decl_var(Parser * parser);

bool parser_match_statement_decl_func(Parser * parser);

bool parser_match_statement_if(Parser * parser);

bool parser_match_statement_for(Parser * parser);

bool parser_match_statement_block(Parser * parser);

// Expressions
bool parser_match_expression(Parser * parser);

// Program
AST_Node * parser_parse_program(Parser * parser);

// Statements
AST_Node * parser_parse_statements(Parser * parser);

AST_Node * parser_parse_statement(Parser * parser);

//AST_Node * parser_parse_statement_decl(Parser * parser);
//
//AST_Node * parser_parse_statement_assign(Parser * parser);

AST_Node * parser_parse_statement_expr(Parser * parser);

AST_Node * parser_parse_statement_decl_var(Parser * parser);

AST_Node * parser_parse_statement_decl_func(Parser * parser);

AST_Node * parser_parse_statement_if(Parser * parser);

AST_Node * parser_parse_statement_for(Parser * parser);

AST_Node * parser_parse_statement_block(Parser * parser);

// Expressions
AST_Node * parser_parse_expression(Parser * parser);

AST_Node * parser_parse_expression_assign(Parser * paser);

AST_Node * parser_parse_expression_equality(Parser * parser);

AST_Node * parser_parse_expression_relational(Parser * parser);

AST_Node * parser_parse_expression_bitshift(Parser * parser);

AST_Node * parser_parse_expression_additive(Parser * parser);

AST_Node * parser_parse_expression_multiplicative(Parser * parser);

AST_Node * parser_parse_expression_prefix(Parser * parser);

AST_Node * parser_parse_expression_postfix(Parser * parser);

AST_Node * parser_parse_expression_factor(Parser * parser);


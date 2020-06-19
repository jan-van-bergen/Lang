#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void parser_init(Parser * parser, Token const * tokens, int token_count) {
	parser->token_count = token_count;
	parser->tokens      = tokens;

	parser->index = 0;
}

//static Token const * parser_curr(Parser const * parser) {
//	return parser->tokens + parser->index;
//}

static bool parser_match(Parser * parser, Token_Type token_type) {
	return parser->tokens[parser->index].type == token_type;
}

static Token const * parser_advance(Parser * parser) {
	char string[128];
	token_to_string(&parser->tokens[parser->index], string, sizeof(string));
	
	printf("%s\n", string);

	return &parser->tokens[parser->index++];
}

static void parser_match_and_advance(Parser * parser, Token_Type token_type) {
	bool match = parser_match(parser, token_type);
	if (!match) {
		char token_string[128];
		token_to_string(parser->tokens + parser->index, token_string, sizeof(token_string));

		printf("ERROR: Unexpected Token '%s'!\n", token_string);
	}

	parser_advance(parser);
}

//void parse_stat_decl(Parser * parser, AST_Statement * stat) {
//	stat->type = AST_STATEMENT_DECL;
//
//	stat->stat_decl.name = parser_peek(parser)->value_str;
//	parser_eat(parser, TOKEN_IDENTIFIER);
//
//	parser_eat(parser, TOKEN_COLON);
//
//	if (parser_peek(parser)->type == TOKEN_IDENTIFIER) {
//		stat->stat_decl.type = parser_peek(parser)->value_str;
//		parser_eat(parser, TOKEN_IDENTIFIER);
//	} else {
//		stat->stat_decl.type = "No Type";
//	}
//
//	if (parser_peek(parser)->type == TOKEN_ASSIGN) {
//		// parse exprsio
//		parser_eat(parser, TOKEN_ASSIGN);
//		//parser_eat(parser, TOKEN_LITERAL_STRING); // temp
//		parser->index++;
//	} else {
//		stat->stat_decl.expr = NULL;
//	}
//	// parser_eat(parser, TOKEN_SEMICOLON);
//}

//void parse_stat_assign(Parser * parser, AST_Statement * stat) {
//	stat->type = AST_STATEMENT_ASSIGN;
//
//	stat->stat_assign.name = parser_peek(parser)->value_str;
//	parser_eat(parser, TOKEN_IDENTIFIER);
//
//	parser_eat(parser, TOKEN_ASSIGN);
//
//	stat->stat_assign.expr = malloc(sizeof(AST_Expression));
//	stat->stat_assign.expr->expr_const.token = *parser_peek(parser); // TEMP
//	parser_eat(parser, TOKEN_LITERAL_INT); // TEMP
//}

//void parse_stat_decl_assign(Parser * parser, AST_Statement * stat) {
//
//}

void parse_program(Parser * parser, AST_Statement * stat) {
	stat->type = AST_STATEMENT_BLOCK;

	stat->stat_block.statement_count = 1;
	stat->stat_block.statements = calloc(stat->stat_block.statement_count, sizeof(AST_Statement));

	parse_statement(parser, &stat->stat_block.statements[0]);

	while (!parser_match(parser, TOKEN_EOF)) {
		stat->stat_block.statement_count++;
		stat->stat_block.statements = realloc(
			stat->stat_block.statements,
			stat->stat_block.statement_count * sizeof(AST_Statement)
		);

		parse_statement(parser, &stat->stat_block.statements[stat->stat_block.statement_count - 1]);
	}
}

void parse_statement(Parser * parser, AST_Statement * stat) {
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		Token const * identifier = parser_advance(parser);

		if (parser_match(parser, TOKEN_COLON)) {
			stat->type = AST_STATEMENT_DECL;

			stat->stat_decl.name = identifier->value_str;
			
			parser_advance(parser);

			if (parser_match(parser, TOKEN_IDENTIFIER)) {
				Token const * type = parser_advance(parser);
				stat->stat_decl.type = type->value_str;
			} else {
				stat->stat_decl.type = NULL;
			}

			if (parser_match(parser, TOKEN_ASSIGN)) {
				parser_advance(parser);

				stat->stat_decl.expr = malloc(sizeof(AST_Expression));
				parse_expression(parser, stat->stat_decl.expr);
			}

			parser_match_and_advance(parser, TOKEN_SEMICOLON);
		} else if (parser_match(parser, TOKEN_ASSIGN)) {
			stat->type = AST_STATEMENT_ASSIGN;
			
			stat->stat_assign.name = identifier->value_str;

			parser_advance(parser);

			stat->stat_assign.expr = malloc(sizeof(AST_Expression));
			parse_expression(parser, stat->stat_assign.expr);
			
			parser_match_and_advance(parser, TOKEN_SEMICOLON);
		}
    } else if (parser_match(parser, TOKEN_KEYWORD_IF)) {
		stat->type = AST_STATEMENT_IF;

		parser_advance(parser);
		parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);

		stat->stat_if.condition = malloc(sizeof(AST_Expression));
		parse_expression(parser, stat->stat_if.condition);

		parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

		stat->stat_if.case_true = malloc(sizeof(AST_Statement));
		parse_statement(parser, stat->stat_if.case_true);

		if (parser_match(parser, TOKEN_KEYWORD_ELSE)) {
			parser_advance(parser);

			stat->stat_if.case_false = malloc(sizeof(AST_Statement));
			parse_statement(parser, stat->stat_if.case_false);
		} else {
			stat->stat_if.case_false = NULL;
		}
	} else if (parser_match(parser, TOKEN_BRACES_OPEN)) {
		stat->type = AST_STATEMENT_BLOCK;
		
		parser_advance(parser);
		
		stat->stat_block.statement_count = 1;
		stat->stat_block.statements = calloc(stat->stat_block.statement_count, sizeof(AST_Statement));

		parse_statement(parser, &stat->stat_block.statements[0]);

		while (!parser_match(parser, TOKEN_BRACES_CLOSE)) {
			stat->stat_block.statement_count++;
			stat->stat_block.statements = realloc(
				stat->stat_block.statements,
				stat->stat_block.statement_count * sizeof(AST_Statement)
			);

			parse_statement(parser, &stat->stat_block.statements[stat->stat_block.statement_count - 1]);
		}

		parser_match_and_advance(parser, TOKEN_BRACES_CLOSE);
	} else {
		puts("Parsing error!");
		abort();
	}
}

void parse_expression(Parser * parser, AST_Expression * expr) {
	if (parser_match(parser, TOKEN_LITERAL_INT) ||
		parser_match(parser, TOKEN_LITERAL_BOOL) ||
		parser_match(parser, TOKEN_LITERAL_STRING)) {
		expr->type = AST_EXPRESSION_CONST;
		expr->expr_const.token = parser->tokens[parser->index];

		parser_advance(parser);
	} else if (parser_match(parser, TOKEN_IDENTIFIER)) {
		expr->type = AST_EXPRESSION_VAR;
		expr->expr_var.token = parser->tokens[parser->index];

		parser_advance(parser);
	} else {
		puts("Parsing error!");
		abort();
	}
}

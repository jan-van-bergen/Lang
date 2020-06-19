#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>

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

bool parser_match_program(Parser * parser) {
	return parser_match_statements(parser);
}

bool parser_match_statements(Parser * parser) {
	return parser_match_statement(parser);
}

bool parser_match_statement(Parser * parser) {
	return
		parser_match_statement_decl(parser)   ||
		parser_match_statement_assign(parser) ||
		parser_match_statement_if(parser);
}

bool parser_match_statement_decl(Parser * parser) {
	return parser_match(parser, TOKEN_IDENTIFIER);
}

bool parser_match_statement_assign(Parser * parser) {
	return parser_match(parser, TOKEN_IDENTIFIER);
}

bool parser_match_statement_if(Parser * parser) {
	return parser_match(parser, TOKEN_KEYWORD_IF);
}

bool parser_match_statement_block(Parser * parser) {
	return parser_match(parser, TOKEN_BRACES_OPEN);
}

bool parser_match_expression_relational(Parser * parser) {
	return parser_match_expression_arithmetic(parser);
}

bool parser_match_expression_arithmetic(Parser * parser) {
	return parser_match_expression_term(parser);
}

bool parser_match_expression_term(Parser * parser) {
	return parser_match_expression_factor(parser);
}

bool parser_match_expression_factor(Parser * parser) {
	return
		parser_match(parser, TOKEN_IDENTIFIER) ||
		parser_match(parser, TOKEN_LITERAL_INT) ||
		parser_match(parser, TOKEN_LITERAL_BOOL) ||
		parser_match(parser, TOKEN_LITERAL_STRING) ||
		parser_match(parser, TOKEN_PARENTESES_OPEN);
}

AST_Node * parser_parse_program(Parser * parser) {
	AST_Node * program = parser_parse_statements(parser);
	parser_match_and_advance(parser, TOKEN_EOF);

	return program;
}

AST_Node * parser_parse_statements(Parser * parser) {
	AST_Node * statement_head = parser_parse_statement(parser);

	if (parser_match_statements(parser)) {
		AST_Node * statement_cons = parser_parse_statements(parser);
		
		AST_Node * statements = malloc(sizeof(AST_Node));
		statements->type = AST_STATEMENTS;
		statements->stat_statements.head = statement_head;
		statements->stat_statements.cons = statement_cons;

		return statements;
	}

	return statement_head;
}

AST_Node * parser_parse_statement(Parser * parser) {
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		Token const * identifier = parser_advance(parser);

		AST_Node * statement = malloc(sizeof(AST_Node));

		if (parser_match(parser, TOKEN_COLON)) {
			parser_advance(parser);

			statement->type = AST_STATEMENT_DECL;
			statement->stat_decl.name = identifier->value_str;
			
			if (parser_match(parser, TOKEN_IDENTIFIER)) {
				Token const * type = parser_advance(parser);
				statement->stat_decl.type = type->value_str;
			} else {
				statement->stat_decl.type = NULL;
			}

			if (parser_match(parser, TOKEN_ASSIGN)) {
				parser_advance(parser);

				statement->stat_decl.expr = parser_parse_expression_relational(parser);
			}
		} else if (parser_match(parser, TOKEN_ASSIGN)) {
			parser_advance(parser);

			statement->type = AST_STATEMENT_ASSIGN;
			statement->stat_assign.name = identifier->value_str;
			statement->stat_assign.expr = parser_parse_expression_relational(parser);
		}
		
		parser_match_and_advance(parser, TOKEN_SEMICOLON);

		return statement;
	} else if (parser_match_statement_if(parser)) {
		return parser_parse_statement_if(parser);
	} else if (parser_match_statement_block(parser)) {
		return parser_parse_statement_block(parser);
	} else {
		abort();
	}
}

AST_Node * parser_parse_statement_decl(Parser * parser) {

}

AST_Node * parser_parse_statement_assign(Parser * parser) {

}

AST_Node * parser_parse_statement_if(Parser * parser) {
	AST_Node * if_statement = malloc(sizeof(AST_Node));
	if_statement->type = AST_STATEMENT_IF;

	parser_match_and_advance(parser, TOKEN_KEYWORD_IF);
	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);

	if_statement->stat_if.condition = parser_parse_expression_relational(parser);

	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	if_statement->stat_if.case_true = parser_parse_statement(parser);

	if (parser_match(parser, TOKEN_KEYWORD_ELSE)) {
		parser_advance(parser);

		if_statement->stat_if.case_false = parser_parse_statement(parser);
	}

	return if_statement;
}

AST_Node * parser_parse_statement_block(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_BRACES_OPEN);

	AST_Node * block = parser_parse_statements(parser);

	parser_match_and_advance(parser, TOKEN_BRACES_CLOSE);

	return block;
}

AST_Node * parser_parse_expression_relational(Parser * parser) {
	AST_Node * arithmetic = parser_parse_expression_arithmetic(parser);

	if (parser_match(parser, TOKEN_OPERATOR_LT) ||
		parser_match(parser, TOKEN_OPERATOR_LT_EQ) ||
		parser_match(parser, TOKEN_OPERATOR_GT) ||
		parser_match(parser, TOKEN_OPERATOR_GT_EQ) ||
		parser_match(parser, TOKEN_OPERATOR_EQ) ||
		parser_match(parser, TOKEN_OPERATOR_NE)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = parser_parse_expression_arithmetic(parser);
		expression->expr_op_bin.expr_right = parser_parse_expression_arithmetic(parser);

		return expression;
	}

	return arithmetic;
}

AST_Node * parser_parse_expression_arithmetic(Parser * parser) {
	AST_Node * term = parser_parse_expression_term(parser);

	if (parser_match(parser, TOKEN_OPERATOR_MULTIPLY) ||
		parser_match(parser, TOKEN_OPERATOR_DIVIDE)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = term;
		expression->expr_op_bin.expr_right = parser_parse_expression_term(parser);

		return expression;
	}

	return term;
}

AST_Node * parser_parse_expression_term(Parser * parser) {
	AST_Node * factor = parser_parse_expression_factor(parser);

	if (parser_match(parser, TOKEN_OPERATOR_PLUS) ||
		parser_match(parser, TOKEN_OPERATOR_MINUS)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = factor;
		expression->expr_op_bin.expr_right = parser_parse_expression_factor(parser);

		return expression;
	}

	return factor;
}

AST_Node * parser_parse_expression_factor(Parser * parser) {
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		AST_Node * factor = malloc(sizeof(AST_Node));

		factor->type == AST_EXPRESSION_VAR;
		factor->expr_var.token = *parser_advance(parser);

		return factor;
	} else if (
		parser_match(parser, TOKEN_LITERAL_INT)  ||
		parser_match(parser, TOKEN_LITERAL_BOOL) ||
		parser_match(parser, TOKEN_LITERAL_STRING)
	) {
		AST_Node * factor = malloc(sizeof(AST_Node));

		factor->type = AST_EXPRESSION_CONST;
		factor->expr_var.token = *parser_advance(parser);

		return factor;
	} else if (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
		parser_advance(parser);

		return parser_parse_expression_relational(parser);
	} else {
		abort();
	}
}

//
//void parse_program(Parser * parser, AST_Node * stat) {
//	stat->type = AST_Node_BLOCK;
//
//	stat->stat_block.statement_count = 1;
//	stat->stat_block.statements = calloc(stat->stat_block.statement_count, sizeof(AST_Node));
//
//	parse_statement(parser, &stat->stat_block.statements[0]);
//
//	while (!parser_match(parser, TOKEN_EOF)) {
//		stat->stat_block.statement_count++;
//		stat->stat_block.statements = realloc(
//			stat->stat_block.statements,
//			stat->stat_block.statement_count * sizeof(AST_Node)
//		);
//
//		parse_statement(parser, &stat->stat_block.statements[stat->stat_block.statement_count - 1]);
//	}
//}
//
//void parse_statement(Parser * parser, AST_Node * stat) {
//	if (parser_match(parser, TOKEN_IDENTIFIER)) {
//		Token const * identifier = parser_advance(parser);
//
//		if (parser_match(parser, TOKEN_COLON)) {
//			stat->type = AST_Node_DECL;
//
//			stat->stat_decl.name = identifier->value_str;
//			
//			parser_advance(parser);
//
//			if (parser_match(parser, TOKEN_IDENTIFIER)) {
//				Token const * type = parser_advance(parser);
//				stat->stat_decl.type = type->value_str;
//			} else {
//				stat->stat_decl.type = NULL;
//			}
//
//			if (parser_match(parser, TOKEN_ASSIGN)) {
//				parser_advance(parser);
//
//				stat->stat_decl.expr = malloc(sizeof(AST_Expression));
//				parse_expression(parser, stat->stat_decl.expr);
//			}
//
//			parser_match_and_advance(parser, TOKEN_SEMICOLON);
//		} else if (parser_match(parser, TOKEN_ASSIGN)) {
//			stat->type = AST_Node_ASSIGN;
//			
//			stat->stat_assign.name = identifier->value_str;
//
//			parser_advance(parser);
//
//			stat->stat_assign.expr = malloc(sizeof(AST_Expression));
//			parse_expression(parser, stat->stat_assign.expr);
//			
//			parser_match_and_advance(parser, TOKEN_SEMICOLON);
//		}
//    } else if (parser_match(parser, TOKEN_KEYWORD_IF)) {
//		stat->type = AST_Node_IF;
//
//		parser_advance(parser);
//		parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);
//
//		stat->stat_if.condition = malloc(sizeof(AST_Expression));
//		parse_expression(parser, stat->stat_if.condition);
//
//		parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);
//
//		stat->stat_if.case_true = malloc(sizeof(AST_Node));
//		parse_statement(parser, stat->stat_if.case_true);
//
//		if (parser_match(parser, TOKEN_KEYWORD_ELSE)) {
//			parser_advance(parser);
//
//			stat->stat_if.case_false = malloc(sizeof(AST_Node));
//			parse_statement(parser, stat->stat_if.case_false);
//		} else {
//			stat->stat_if.case_false = NULL;
//		}
//	} else if (parser_match(parser, TOKEN_BRACES_OPEN)) {
//		stat->type = AST_Node_BLOCK;
//		
//		parser_advance(parser);
//		
//		stat->stat_block.statement_count = 1;
//		stat->stat_block.statements = calloc(stat->stat_block.statement_count, sizeof(AST_Node));
//
//		parse_statement(parser, &stat->stat_block.statements[0]);
//
//		while (!parser_match(parser, TOKEN_BRACES_CLOSE)) {
//			stat->stat_block.statement_count++;
//			stat->stat_block.statements = realloc(
//				stat->stat_block.statements,
//				stat->stat_block.statement_count * sizeof(AST_Node)
//			);
//
//			parse_statement(parser, &stat->stat_block.statements[stat->stat_block.statement_count - 1]);
//		}
//
//		parser_match_and_advance(parser, TOKEN_BRACES_CLOSE);
//	} else {
//		puts("Parsing error!");
//		abort();
//	}
//}
//
//void parse_expression(Parser * parser, AST_Expression * expr) {
//	if (parser_match(parser, TOKEN_LITERAL_INT) ||
//		parser_match(parser, TOKEN_LITERAL_BOOL) ||
//		parser_match(parser, TOKEN_LITERAL_STRING)) {
//		expr->type = AST_EXPRESSION_CONST;
//		expr->expr_const.token = parser->tokens[parser->index];
//
//		parser_advance(parser);
//	} else if (parser_match(parser, TOKEN_IDENTIFIER)) {
//		expr->type = AST_EXPRESSION_VAR;
//		expr->expr_var.token = parser->tokens[parser->index];
//
//		parser_advance(parser);
//	} else {
//		puts("Parsing error!");
//		abort();
//	}
//}

#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>

void parser_init(Parser * parser, Token const * tokens, int token_count) {
	parser->token_count = token_count;
	parser->tokens      = tokens;

	parser->index = 0;
}

static void parser_error(Parser * parser) {
	Token const * token = parser->tokens + parser->index;

	char token_string[128];
	token_to_string(token, token_string, sizeof(token_string));

	printf("ERROR: Unexpected Token '%s' at line %i!\n", token_string, token->line);
	abort();
}

static bool parser_match(Parser * parser, Token_Type token_type) {
	return parser->tokens[parser->index].type == token_type;
}

static Token const * parser_advance(Parser * parser) {
	char string[128];
	token_to_string(&parser->tokens[parser->index], string, sizeof(string));
	
	printf("%s\n", string);

	return &parser->tokens[parser->index++];
}

static Token const * parser_match_and_advance(Parser * parser, Token_Type token_type) {
	bool match = parser_match(parser, token_type);
	if (!match) {
		parser_error(parser);
	}

	return parser_advance(parser);
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
		parser_match_statement_func(parser)   ||
		parser_match_statement_if(parser)     ||
		parser_match_statement_for(parser)    ||
		parser_match_statement_block(parser);
}

bool parser_match_statement_decl(Parser * parser) {
	return parser_match(parser, TOKEN_IDENTIFIER);
}

bool parser_match_statement_assign(Parser * parser) {
	return parser_match(parser, TOKEN_IDENTIFIER);
}

bool parser_match_statement_func(Parser * parser) {
	return parser_match(parser, TOKEN_KEYWORD_FUNC);
}

bool parser_match_statement_if(Parser * parser) {
	return parser_match(parser, TOKEN_KEYWORD_IF);
}

bool parser_match_statement_for(Parser * parser) {
	return parser_match(parser, TOKEN_KEYWORD_FOR);
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

				statement->stat_decl.expr = parser_parse_expression(parser);
			} else {
				statement->stat_decl.expr = NULL;
			}
		} else if (parser_match(parser, TOKEN_ASSIGN)) {
			parser_advance(parser);

			statement->type = AST_STATEMENT_ASSIGN;
			statement->stat_assign.name = identifier->value_str;
			statement->stat_assign.expr = parser_parse_expression(parser);
		}
		
		parser_match_and_advance(parser, TOKEN_SEMICOLON);

		return statement;
	} else if (parser_match_statement_func(parser)) {
		return parser_parse_statement_func(parser);
	} else if (parser_match_statement_if(parser)) {
		return parser_parse_statement_if(parser);
	} else if (parser_match_statement_for(parser)) {
		return parser_parse_statement_for(parser);
	} else if (parser_match_statement_block(parser)) {
		return parser_parse_statement_block(parser);
	} else {
		parser_error(parser);
	}
}

//AST_Node * parser_parse_statement_decl(Parser * parser) {
//
//}
//
//AST_Node * parser_parse_statement_assign(Parser * parser) {
//
//}

static AST_Node * parser_parse_arg(Parser * parser) {
	AST_Node * arg = malloc(sizeof(AST_Node));
	arg->type = AST_ARGS;

	arg->args.name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	parser_match_and_advance(parser, TOKEN_COLON);
	arg->args.type = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	arg->args.next = NULL;

	return arg;
}

static AST_Node * parser_parse_args(Parser * parser) {
	AST_Node * args_head = NULL;

	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);
	
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		args_head = parser_parse_arg(parser);
	}
	
	AST_Node * args_curr = args_head;

	while (parser_match(parser, TOKEN_COMMA)) {
		parser_advance(parser);

		AST_Node * arg = parser_parse_arg(parser);
		args_curr->args.next = arg;
		args_curr = arg;
	}
		
	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	return args_head;
}

AST_Node * parser_parse_statement_func(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_KEYWORD_FUNC);

	AST_Node * func = malloc(sizeof(AST_Node));
	func->type = AST_STATEMENT_FUNC;
	func->stat_func.name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	func->stat_func.args = parser_parse_args(parser);
	func->stat_func.body = parser_parse_statement_block(parser);

	return func;
}

AST_Node * parser_parse_statement_if(Parser * parser) {
	AST_Node * branch = malloc(sizeof(AST_Node));
	branch->type = AST_STATEMENT_IF;

	parser_match_and_advance(parser, TOKEN_KEYWORD_IF);
	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);

	branch->stat_if.condition = parser_parse_expression(parser);

	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	branch->stat_if.case_true = parser_parse_statement(parser);

	if (parser_match(parser, TOKEN_KEYWORD_ELSE)) {
		parser_advance(parser);

		branch->stat_if.case_false = parser_parse_statement(parser);
	} else {
		branch->stat_if.case_false = NULL;
	}

	return branch;
}

AST_Node * parser_parse_statement_for(Parser * parser) {
	AST_Node * for_loop = malloc(sizeof(AST_Node));
	for_loop->type = AST_STATEMENT_FOR;
	
	parser_match_and_advance(parser, TOKEN_KEYWORD_FOR);
	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);

	for_loop->stat_for.expr_init = parser_parse_expression(parser);
	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	for_loop->stat_for.expr_condition = parser_parse_expression(parser);
	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	for_loop->stat_for.expr_next = parser_parse_expression(parser);

	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	for_loop->stat_for.body = parser_parse_statement(parser);

	return for_loop;
}

AST_Node * parser_parse_statement_block(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_BRACES_OPEN);

	AST_Node * block = NULL;
	
	if (parser_match_statements(parser)) {
		block = parser_parse_statements(parser);
	}

	parser_match_and_advance(parser, TOKEN_BRACES_CLOSE);

	return block;
}

AST_Node * parser_parse_expression(Parser * parser) {
	return parser_parse_expression_equality(parser);
}

AST_Node * parser_parse_expression_equality(Parser * parser) {
	AST_Node * arithmetic = parser_parse_expression_relational(parser);

	while (
		parser_match(parser, TOKEN_OPERATOR_EQ)    ||
		parser_match(parser, TOKEN_OPERATOR_NE)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = arithmetic;
		expression->expr_op_bin.expr_right = parser_parse_expression_relational(parser);

		arithmetic = expression;
	}

	return arithmetic;
}

AST_Node * parser_parse_expression_relational(Parser * parser) {
	AST_Node * lhs = parser_parse_expression_bitshift(parser);

	while (
		parser_match(parser, TOKEN_OPERATOR_LT)    ||
		parser_match(parser, TOKEN_OPERATOR_LT_EQ) ||
		parser_match(parser, TOKEN_OPERATOR_GT)    ||
		parser_match(parser, TOKEN_OPERATOR_GT_EQ)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_bitshift(parser);

		lhs = expression;
	}

	return lhs;
}

AST_Node * parser_parse_expression_bitshift(Parser * parser) {
	AST_Node * lhs = parser_parse_expression_additive(parser);

	while (
		parser_match(parser, TOKEN_OPERATOR_SHIFT_LEFT) ||
		parser_match(parser, TOKEN_OPERATOR_SHIFT_RIGHT)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_additive(parser);

		lhs = expression;
	}

	return lhs;
}

AST_Node * parser_parse_expression_additive(Parser * parser) {
	AST_Node * lhs = parser_parse_expression_multiplicative(parser);

	while (
		parser_match(parser, TOKEN_OPERATOR_PLUS) ||
		parser_match(parser, TOKEN_OPERATOR_MINUS)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_multiplicative(parser);

		lhs = expression;
	}

	return lhs;
}

AST_Node * parser_parse_expression_multiplicative(Parser * parser) {
	AST_Node * lhs = parser_parse_expression_prefix(parser);

	while (
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY) ||
		parser_match(parser, TOKEN_OPERATOR_DIVIDE)
	) {
		AST_Node * expression = malloc(sizeof(AST_Node));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_prefix(parser);

		lhs = expression;
	}

	return lhs;
}

AST_Node * parser_parse_expression_prefix(Parser * parser) {
	if (parser_match(parser, TOKEN_OPERATOR_INC) ||
		parser_match(parser, TOKEN_OPERATOR_DEC) ||
		parser_match(parser, TOKEN_OPERATOR_PLUS) ||
		parser_match(parser, TOKEN_OPERATOR_MINUS)
	) {
		AST_Node * prefix = malloc(sizeof(AST_Node));
		prefix->type = AST_EXPRESSION_OPERATOR_PRE;
		prefix->expr_op_pre.token = *parser_advance(parser);
		prefix->expr_op_pre.expr  = parser_parse_expression_postfix(parser);

		return prefix;
	}

	return parser_parse_expression_postfix(parser);
}

AST_Node * parser_parse_expression_postfix(Parser * parser) {
	AST_Node * operand =  parser_parse_expression_factor(parser);

	if (parser_match(parser, TOKEN_OPERATOR_INC) ||
		parser_match(parser, TOKEN_OPERATOR_DEC)
	) {
		AST_Node * postfix = malloc(sizeof(AST_Node));
		postfix->type = AST_EXPRESSION_OPERATOR_POST;
		postfix->expr_op_post.token = *parser_advance(parser);
		postfix->expr_op_post.expr  = operand;

		return postfix;
	}

	return operand;
}

AST_Node * parser_parse_expression_factor(Parser * parser) {
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		AST_Node * factor = malloc(sizeof(AST_Node));

		factor->type = AST_EXPRESSION_VAR;
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

		AST_Node * expr = parser_parse_expression(parser);

		parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

		return expr;
	} else {
		parser_error(parser);
	}
}

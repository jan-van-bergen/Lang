#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

void parser_init(Parser * parser, Token const * tokens, int token_count) {
	parser->token_count = token_count;
	parser->tokens      = tokens;

	parser->index = 0;
}

static void parser_error(Parser * parser) {
	Token const * token = parser->tokens + parser->index;

	char token_string[128];
	token_to_string(token, token_string, sizeof(token_string));

	printf("ERROR: Invalid Token '%s' at line %i!\n", token_string, token->line);
	abort();
}

static void parser_error_expected(Parser * parser, Token_Type expected) {
	Token const * token = parser->tokens + parser->index;

	char token_string[128];
	token_to_string(token, token_string, sizeof(token_string));
	
	Token expected_token;
	expected_token.type = expected;
	expected_token.value_str = "identifier";
	char expected_token_string[128];
	token_to_string(&expected_token, expected_token_string, sizeof(expected_token_string));

	printf("ERROR: Unexpected Token '%s' at line %i! Expected: '%s'\n", token_string, token->line, expected_token_string);
	abort();
}

static bool parser_match(Parser const * parser, Token_Type token_type) {
	return parser->tokens[parser->index].type == token_type;
}

static Token const * parser_advance(Parser * parser) {
	//char string[128];
	//token_to_string(&parser->tokens[parser->index], string, sizeof(string));
	//
	//printf("%s\n", string);

	return &parser->tokens[parser->index++];
}

static Token const * parser_match_and_advance(Parser * parser, Token_Type token_type) {
	bool match = parser_match(parser, token_type);
	if (!match) {
		parser_error_expected(parser, token_type);
	}

	return parser_advance(parser);
}

static bool parser_match_expression(Parser const * parser) {
	return
		parser_match(parser, TOKEN_IDENTIFIER) || // Variable
		parser_match(parser, TOKEN_OPERATOR_PLUS)        || // Unary operations
		parser_match(parser, TOKEN_OPERATOR_MINUS)       ||
		parser_match(parser, TOKEN_OPERATOR_INC)         ||
		parser_match(parser, TOKEN_OPERATOR_DEC)         ||
		parser_match(parser, TOKEN_OPERATOR_BITWISE_AND) ||
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY)    ||
		parser_match(parser, TOKEN_LITERAL_INT)    || // Constant values
		parser_match(parser, TOKEN_LITERAL_BOOL)   ||
		parser_match(parser, TOKEN_LITERAL_STRING) ||
		parser_match(parser, TOKEN_PARENTESES_OPEN); // Subexpression
}

static bool parser_match_statement_expr(Parser const * parser) {
	return parser_match_expression(parser);
}

static bool parser_match_statement_decl_var(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_LET);
}

static bool parser_match_statement_decl_func(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_FUNC);
}

static bool parser_match_statement_extern(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_EXTERN);
}

static bool parser_match_statement_if(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_IF);
}

static bool parser_match_statement_while(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_WHILE);
}

static bool parser_match_statement_break(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_BREAK);
}

static bool parser_match_statement_continue(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_CONTINUE);
}

static bool parser_match_statement_return(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_RETURN);
}

static bool parser_match_statement_block(Parser const * parser) {
	return parser_match(parser, TOKEN_BRACES_OPEN);
}

static bool parser_match_statement(Parser const * parser) {
	return
		parser_match_statement_expr(parser) ||
		parser_match_statement_decl_var (parser) ||
		parser_match_statement_decl_func(parser) ||
		parser_match_statement_extern   (parser) ||
		parser_match_statement_if   (parser) ||
		parser_match_statement_while(parser) ||
		parser_match_statement_break   (parser) ||
		parser_match_statement_continue(parser) ||
		parser_match_statement_return  (parser) ||
		parser_match_statement_block(parser);
}

static bool parser_match_program(Parser const * parser) {
	return parser_match_statement(parser);
}

static AST_Expression * parser_parse_expression(Parser * parser);

static AST_Call_Arg * parser_parse_call_arg(Parser * parser) {
	AST_Call_Arg * arg = malloc(sizeof(AST_Call_Arg));
	arg->expr = parser_parse_expression(parser);
	arg->next = NULL;
	arg->height = arg->expr->height;

	return arg;
}

static AST_Call_Arg * parser_parse_call_args(Parser * parser) {
	AST_Call_Arg * args_head = NULL;

	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);
	
	if (parser_match_expression(parser)) {
		args_head = parser_parse_call_arg(parser);
	}
	
	AST_Call_Arg * args_curr = args_head;

	while (parser_match(parser, TOKEN_COMMA)) {
		parser_advance(parser);

		AST_Call_Arg * arg = parser_parse_call_arg(parser);
		args_curr->next = arg;
		args_curr       = arg;

		args_head->height = max(args_head->height, arg->height);
	}
		
	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	return args_head;
}

static AST_Expression * parser_parse_expression_elementary(Parser * parser) {
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		Token const * identifier = parser_advance(parser);

		if (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
			AST_Expression * expr = malloc(sizeof(AST_Expression));

			expr->type = AST_EXPRESSION_CALL_FUNC;
			expr->expr_call.function = identifier->value_str;
			expr->expr_call.args = parser_parse_call_args(parser);
			expr->height = expr->expr_call.args->height;

			return expr;
		} else {
			AST_Expression * expr = malloc(sizeof(AST_Expression));

			expr->type = AST_EXPRESSION_VAR;
			expr->height = 0;
			expr->expr_var.token = *identifier;

			return expr;
		}
	} else if (
		parser_match(parser, TOKEN_LITERAL_INT)  ||
		parser_match(parser, TOKEN_LITERAL_BOOL) ||
		parser_match(parser, TOKEN_LITERAL_STRING)
	) {
		AST_Expression * factor = malloc(sizeof(AST_Expression));

		factor->type = AST_EXPRESSION_CONST;
		factor->height = 0;
		factor->expr_var.token = *parser_advance(parser);

		return factor;
	} else if (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
		parser_advance(parser);

		AST_Expression * expr = parser_parse_expression(parser);

		parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

		return expr;
	} else {
		parser_error(parser);
	}
}

static AST_Expression * parser_parse_expression_postfix(Parser * parser) {
	AST_Expression * operand =  parser_parse_expression_elementary(parser);

	if (parser_match(parser, TOKEN_OPERATOR_INC) ||
		parser_match(parser, TOKEN_OPERATOR_DEC)
	) {
		AST_Expression * postfix = malloc(sizeof(AST_Expression));
		postfix->type = AST_EXPRESSION_OPERATOR_POST;
		postfix->height = operand->height + 1;
		postfix->expr_op_post.token = *parser_advance(parser);
		postfix->expr_op_post.expr  = operand;

		return postfix;
	}

	return operand;
}

static AST_Expression * parser_parse_expression_prefix(Parser * parser) {
	if (parser_match(parser, TOKEN_OPERATOR_INC) ||
		parser_match(parser, TOKEN_OPERATOR_DEC) ||
		parser_match(parser, TOKEN_OPERATOR_PLUS)  ||
		parser_match(parser, TOKEN_OPERATOR_MINUS) ||
		parser_match(parser, TOKEN_OPERATOR_BITWISE_AND) || // address of
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY)       // dereference
	) {
		AST_Expression * prefix = malloc(sizeof(AST_Expression));
		prefix->type = AST_EXPRESSION_OPERATOR_PRE;
		prefix->expr_op_pre.token = *parser_advance(parser);
		prefix->expr_op_pre.expr  = parser_parse_expression_prefix(parser);
		prefix->height = prefix->expr_op_pre.expr->height + 1;

		return prefix;
	}

	return parser_parse_expression_postfix(parser);
}

static AST_Expression * parser_parse_expression_multiplicative(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_prefix(parser);
	
	// Left Associative
	while (
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY) ||
		parser_match(parser, TOKEN_OPERATOR_DIVIDE) ||
		parser_match(parser, TOKEN_OPERATOR_MODULO)
	) {
		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_prefix(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_additive(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_multiplicative(parser);
	
	// Left Associative
	while (
		parser_match(parser, TOKEN_OPERATOR_PLUS) ||
		parser_match(parser, TOKEN_OPERATOR_MINUS)
	) {
		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_multiplicative(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_bitshift(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_additive(parser);
	
	// Left Associative
	while (
		parser_match(parser, TOKEN_OPERATOR_SHIFT_LEFT) ||
		parser_match(parser, TOKEN_OPERATOR_SHIFT_RIGHT)
	) {
		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_additive(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_relational(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_bitshift(parser);
	
	// Left Associative
	while (
		parser_match(parser, TOKEN_OPERATOR_LT)    ||
		parser_match(parser, TOKEN_OPERATOR_LT_EQ) ||
		parser_match(parser, TOKEN_OPERATOR_GT)    ||
		parser_match(parser, TOKEN_OPERATOR_GT_EQ)
	) {
		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_bitshift(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_equality(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_relational(parser);

	// Left Associative
	while (
		parser_match(parser, TOKEN_OPERATOR_EQ) ||
		parser_match(parser, TOKEN_OPERATOR_NE)
	) {
		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_relational(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_assign(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_equality(parser);

	// Right Associative
	if (parser_match(parser, TOKEN_ASSIGN)) {
		if (lhs->type != AST_EXPRESSION_VAR && lhs->type != AST_EXPRESSION_OPERATOR_PRE) {
			printf("ERROR: Left hand operand of '=' operator must be a variable!\n");
			abort();
		}

		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression(Parser * parser) {
	return parser_parse_expression_assign(parser);
}

static AST_Statement * parser_parse_statement      (Parser * parser); 
static AST_Statement * parser_parse_statements     (Parser * parser); 
static AST_Statement * parser_parse_statement_block(Parser * parser); 

static AST_Statement * parser_parse_statement_expr(Parser * parser) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));

	stat->type = AST_STATEMENT_EXPR;
	stat->stat_expr.expr = parser_parse_expression(parser);

	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	return stat;
}

static AST_Statement * parser_parse_statement_decl_var(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_KEYWORD_LET);

	AST_Statement * decl = malloc(sizeof(AST_Statement));
	decl->type = AST_STATEMENT_DECL_VAR;

	decl->stat_decl_var.name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	parser_match_and_advance(parser, TOKEN_COLON);
	decl->stat_decl_var.type = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;

	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	return decl;
}

static AST_Decl_Arg * parser_parse_decl_arg(Parser * parser) {
	AST_Decl_Arg * arg = malloc(sizeof(AST_Decl_Arg));

	arg->name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	parser_match_and_advance(parser, TOKEN_COLON);
	arg->type = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	arg->next = NULL;

	return arg;
}

static AST_Decl_Arg * parser_parse_decl_args(Parser * parser) {
	AST_Decl_Arg * args_head = NULL;

	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);
	
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		args_head = parser_parse_decl_arg(parser);
	}
	
	AST_Decl_Arg * args_curr = args_head;

	while (parser_match(parser, TOKEN_COMMA)) {
		parser_advance(parser);

		AST_Decl_Arg * arg = parser_parse_decl_arg(parser);
		args_curr->next = arg;
		args_curr       = arg;
	}
		
	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	return args_head;
}

static AST_Statement * parser_parse_statement_decl_func(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_KEYWORD_FUNC);

	AST_Statement * func = malloc(sizeof(AST_Statement));
	func->type = AST_STATEMENT_DECL_FUNC;
	func->stat_decl_func.name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	func->stat_decl_func.args = parser_parse_decl_args(parser);

	if (parser_match(parser, TOKEN_ARROW)) {
		parser_advance(parser);

		func->stat_decl_func.return_type = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	} else {
		char const * void_str     = "void";
		int          void_str_len = strlen(void_str);

		func->stat_decl_func.return_type = malloc(void_str_len + 1);
		strcpy_s(func->stat_decl_func.return_type, void_str_len + 1, void_str);
	}

	func->stat_decl_func.body = parser_parse_statement_block(parser);

	return func;
}

static AST_Statement * parser_parse_statement_extern(Parser * parser) {
	AST_Statement * decl_extern = malloc(sizeof(AST_Statement));
	decl_extern->type = AST_STATEMENT_EXTERN;

	parser_match_and_advance(parser, TOKEN_KEYWORD_EXTERN);
	decl_extern->stat_extern.name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;

	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	return decl_extern;
}

static AST_Statement * parser_parse_statement_if(Parser * parser) {
	AST_Statement * branch = malloc(sizeof(AST_Statement));
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

static AST_Statement * parser_parse_statement_while(Parser * parser) {
	AST_Statement * while_loop = malloc(sizeof(AST_Statement));
	while_loop->type = AST_STATEMENT_WHILE;
	
	parser_match_and_advance(parser, TOKEN_KEYWORD_WHILE);
	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);

	while_loop->stat_while.condition = parser_parse_expression(parser);

	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	while_loop->stat_while.body = parser_parse_statement(parser);

	return while_loop;
}

static AST_Statement * parser_parse_statement_break(Parser * parser) {
	AST_Statement * ret = malloc(sizeof(AST_Statement));
	ret->type = AST_STATEMENT_BREAK;
	
	parser_match_and_advance(parser, TOKEN_KEYWORD_BREAK);
	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	return ret;
}

static AST_Statement * parser_parse_statement_continue(Parser * parser) {
	AST_Statement * ret = malloc(sizeof(AST_Statement));
	ret->type = AST_STATEMENT_CONTINUE;
	
	parser_match_and_advance(parser, TOKEN_KEYWORD_CONTINUE);
	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	return ret;
}

static AST_Statement * parser_parse_statement_return(Parser * parser) {
	AST_Statement * ret = malloc(sizeof(AST_Statement));
	ret->type = AST_STATEMENT_RETURN;
	
	parser_match_and_advance(parser, TOKEN_KEYWORD_RETURN);

	if (!parser_match(parser, TOKEN_SEMICOLON)) {
		ret->stat_return.expr = parser_parse_expression(parser);
	} else {
		ret->stat_return.expr = NULL;
	}

	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	return ret;
}

static AST_Statement * parser_parse_statement_block(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_BRACES_OPEN);

	AST_Statement * block;
	
	if (parser_match_statement(parser)) {
		block = parser_parse_statements(parser);
	} else {
		block = malloc(sizeof(AST_Statement));
		block->type = AST_STATEMENT_NOOP;
	}

	parser_match_and_advance(parser, TOKEN_BRACES_CLOSE);

	return block;
}

static AST_Statement * parser_parse_statement(Parser * parser) {
	if (parser_match_expression(parser)) {
		return parser_parse_statement_expr(parser);
	} else if (parser_match_statement_decl_var(parser)) {
		return parser_parse_statement_decl_var(parser);
	} else if (parser_match_statement_decl_func(parser)) {
		return parser_parse_statement_decl_func(parser);
	} else if (parser_match_statement_extern(parser)) {
		return parser_parse_statement_extern(parser);
	} else if (parser_match_statement_if(parser)) {
		return parser_parse_statement_if(parser);
	} else if (parser_match_statement_while(parser)) {
		return parser_parse_statement_while(parser);
	} else if (parser_match_statement_break(parser)) {
		return parser_parse_statement_break(parser);
	} else if (parser_match_statement_continue(parser)) {
		return parser_parse_statement_continue(parser);
	} else if (parser_match_statement_return(parser)) {
		return parser_parse_statement_return(parser);
	} else if (parser_match_statement_block(parser)) {
		return parser_parse_statement_block(parser);
	} else {
		parser_error(parser);
	}
}

static AST_Statement * parser_parse_statements(Parser * parser) {
	AST_Statement * statement_head = parser_parse_statement(parser);

	if (parser_match_statement(parser)) {
		AST_Statement * statement_cons = parser_parse_statements(parser);
		
		AST_Statement * statements = malloc(sizeof(AST_Statement));
		statements->type = AST_STATEMENTS;
		statements->stat_stats.head = statement_head;
		statements->stat_stats.cons = statement_cons;

		return statements;
	}

	return statement_head;
}

AST_Statement * parser_parse_program(Parser * parser) {
	AST_Statement * program = parser_parse_statements(parser);
	parser_match_and_advance(parser, TOKEN_EOF);

	return program;
}

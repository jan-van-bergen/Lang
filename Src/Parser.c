#include "Parser.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "Error.h"

void parser_init(Parser * parser, int token_count, Token const * tokens) {
	parser->token_count = token_count;
	parser->tokens      = tokens;

	parser->index = 0;
	parser->current_line = 0;

	parser->current_variable_buffer = NULL;
	parser->current_scope           = NULL;
}

static void parser_error(Parser * parser) {
	Token const * token = parser->tokens + parser->index;

	char token_string[128];
	token_to_string(token, token_string, sizeof(token_string));

	error(ERROR_PARSER, "Invalid Token '%s' at line %i!\n", token_string, token->line);
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

	error_set_line(token->line);
	error(ERROR_PARSER, "Unexpected Token '%s'! Expected: '%s'\n", token_string, expected_token_string);
}

static bool parser_match(Parser const * parser, Token_Type token_type) {
	return parser->tokens[parser->index].type == token_type;
}

static bool parser_matchn(Parser const * parser, Token_Type token_type, int offset) {
	if (parser->index + offset < parser->token_count) {
		return parser->tokens[parser->index + offset].type == token_type;
	} else {
		return false;
	}
}


static void parser_push_scope(Parser * parser, Scope * scope) {
	scope->prev = parser->current_scope;
	parser->current_scope = scope;
}

static void parser_pop_scope(Parser * parser) {
	parser->current_scope = parser->current_scope->prev;
}


static Token const * parser_advance(Parser * parser) {
	Token const * token = &parser->tokens[parser->index++];
	parser->current_line = token->line;
	error_set_line(parser->current_line);
	return token;
}

static Token const * parser_expect(Parser * parser, Token_Type token_type) {
	bool match = parser_match(parser, token_type);
	if (!match) {
		parser_error_expected(parser, token_type);
	}

	return parser_advance(parser);
}

static bool parser_match_type(Parser const * parser) {
	return 
		parser_match(parser, TOKEN_PARENTESES_OPEN) ||
		parser_match(parser, TOKEN_KEYWORD_VOID)   ||
		parser_match(parser, TOKEN_KEYWORD_BOOL)   ||
		parser_match(parser, TOKEN_KEYWORD_CHAR)   ||
		parser_match(parser, TOKEN_KEYWORD_INT)    ||
		parser_match(parser, TOKEN_KEYWORD_FLOAT)  ||
		parser_match(parser, TOKEN_KEYWORD_DOUBLE) ||
		parser_match(parser, TOKEN_KEYWORD_I8)  ||
		parser_match(parser, TOKEN_KEYWORD_I16) ||
		parser_match(parser, TOKEN_KEYWORD_I32) ||
		parser_match(parser, TOKEN_KEYWORD_I64) ||
		parser_match(parser, TOKEN_KEYWORD_U8)  ||
		parser_match(parser, TOKEN_KEYWORD_U16) ||
		parser_match(parser, TOKEN_KEYWORD_U32) ||
		parser_match(parser, TOKEN_KEYWORD_U64) ||
		parser_match(parser, TOKEN_KEYWORD_F32) ||
		parser_match(parser, TOKEN_KEYWORD_F64) ||
		parser_match(parser, TOKEN_IDENTIFIER);
}

static bool parser_match_expression(Parser const * parser) {
	return
		parser_match(parser, TOKEN_IDENTIFIER) || // Variable
		parser_match(parser, TOKEN_KEYWORD_CAST)   ||
		parser_match(parser, TOKEN_KEYWORD_SIZEOF) ||
		parser_match(parser, TOKEN_OPERATOR_PLUS)        || // Unary operations
		parser_match(parser, TOKEN_OPERATOR_MINUS)       ||
		parser_match(parser, TOKEN_OPERATOR_INC)         ||
		parser_match(parser, TOKEN_OPERATOR_DEC)         ||
		parser_match(parser, TOKEN_OPERATOR_BITWISE_AND) ||
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY)    ||
		parser_match(parser, TOKEN_OPERATOR_BITWISE_NOT) ||
		parser_match(parser, TOKEN_OPERATOR_LOGICAL_NOT) ||
		parser_match(parser, TOKEN_LITERAL_INT)    || // Constant values
		parser_match(parser, TOKEN_LITERAL_F32)    ||
		parser_match(parser, TOKEN_LITERAL_F64)    ||
		parser_match(parser, TOKEN_LITERAL_BOOL)   ||
		parser_match(parser, TOKEN_LITERAL_CHAR)   ||
		parser_match(parser, TOKEN_LITERAL_STRING) ||
		parser_match(parser, TOKEN_KEYWORD_NULL)   ||
		parser_match(parser, TOKEN_PARENTESES_OPEN); // Subexpression
}

static bool parser_match_statement_expr(Parser const * parser) {
	return parser_match_expression(parser);
}

static bool parser_match_statement_def_var(Parser const * parser) {
	return parser_match(parser, TOKEN_IDENTIFIER) && parser_matchn(parser, TOKEN_COLON, 1);
}

static bool parser_match_statement_def_func(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_FUNC);
}

static bool parser_match_statement_def_struct(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_STRUCT);
}

static bool parser_match_statement_extern(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_EXTERN);
}

static bool parser_match_statement_export(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_EXPORT);
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
		parser_match_statement_def_var   (parser) ||
		parser_match_statement_def_func  (parser) ||
		parser_match_statement_def_struct(parser) ||
		parser_match_statement_extern(parser) ||
		parser_match_statement_export(parser) ||
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

static Type const * parser_parse_type(Parser * parser) {
	Type * type = type_table_new_type();
	type->base = NULL;

	if (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
		parser_advance(parser);

		int num_extra_parens = 0;
		while (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
			parser_advance(parser);
			num_extra_parens++;
		}

		int     arg_count = 0;
		Type ** arg_types = NULL;

		// Parse arguments (if any)
		if (!parser_match(parser, TOKEN_PARENTESES_CLOSE)) {
			int arg_types_cap = 16;
			arg_types = mem_alloc(arg_types_cap * sizeof(Type *));

			arg_types[arg_count++] = parser_parse_type(parser);

			while (parser_match(parser, TOKEN_COMMA)) {
				parser_advance(parser);

				if (arg_count == arg_types_cap) {
					arg_types_cap *= 2;
					arg_types = mem_realloc(arg_types, arg_types_cap);
				}

				arg_types[arg_count++] = parser_parse_type(parser);
			}
		}

		parser_expect(parser, TOKEN_PARENTESES_CLOSE);

		// Parse return type
		Type const * return_type = NULL;

		if (parser_match(parser, TOKEN_ARROW)) {
			parser_advance(parser);
			return_type = parser_parse_type(parser);
		} else {
			return_type = make_type_void();
		}

		for (int i = 0; i < num_extra_parens; i++) {
			parser_expect(parser, TOKEN_PARENTESES_CLOSE);
		}

		type = make_type_function(arg_types, arg_count, return_type);
	} else {
		if (parser_match(parser, TOKEN_KEYWORD_VOID)) {
			type->type = TYPE_VOID;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_BOOL)) {
			type->type = TYPE_BOOL;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_I8)) {
			type->type = TYPE_I8;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_I16)) {
			type->type = TYPE_I16;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_I32) || parser_match(parser, TOKEN_KEYWORD_INT)) {
			type->type = TYPE_I32;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_I64)) {
			type->type = TYPE_I64;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_U8) || parser_match(parser, TOKEN_KEYWORD_CHAR)) {
			type->type = TYPE_U8;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_U16)) {
			type->type = TYPE_U16;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_U32)) {
			type->type = TYPE_U32;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_U64)) {
			type->type = TYPE_U64;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_F32) || parser_match(parser, TOKEN_KEYWORD_FLOAT)) {
			type->type = TYPE_F32;
			parser_advance(parser);
		} else if (parser_match(parser, TOKEN_KEYWORD_F64) || parser_match(parser, TOKEN_KEYWORD_DOUBLE)) {
			type->type = TYPE_F64;
			parser_advance(parser);
		} else {
			char const * identifier = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;

			type->type        = TYPE_STRUCT;
			type->struct_name = identifier;
		}

		while (
			parser_match(parser, TOKEN_OPERATOR_MULTIPLY) ||
			parser_match(parser, TOKEN_SQUARE_BRACES_OPEN)
		) {
			if (parser_match(parser, TOKEN_OPERATOR_MULTIPLY)) {
				parser_advance(parser);

				type = make_type_pointer(type);
			} else if (parser_match(parser, TOKEN_SQUARE_BRACES_OPEN)) {
				parser_advance(parser);

				int array_size = parser_expect(parser, TOKEN_LITERAL_INT)->value_int;
				parser_expect(parser, TOKEN_SQUARE_BRACES_CLOSE);

				type = make_type_array(type, array_size);
			}
		}
	}

	return type;
}

static AST_Expression * parser_parse_expression(Parser * parser);

static void parser_parse_call_arg(Parser * parser, AST_Call_Arg * arg) {
	arg->expr   = parser_parse_expression(parser);
	arg->height = arg->expr->height;
}

static AST_Call_Arg * parser_parse_call_args(Parser * parser, int * arg_count) {
	*arg_count = 0;

	int arg_capacity = 16;
	AST_Call_Arg * args = mem_alloc(arg_capacity * sizeof(AST_Call_Arg));

	parser_expect(parser, TOKEN_PARENTESES_OPEN);
	
	if (parser_match_expression(parser)) {
		parser_parse_call_arg(parser, args + (*arg_count)++);
	}
	
	while (parser_match(parser, TOKEN_COMMA)) {
		parser_advance(parser);

		if (*arg_count == arg_capacity) {
			arg_capacity *= 2;
			args = mem_realloc(args, arg_capacity * sizeof(AST_Call_Arg));
		}

		AST_Call_Arg * arg = args + (*arg_count)++;
		parser_parse_call_arg(parser, arg);
	}

	parser_expect(parser, TOKEN_PARENTESES_CLOSE);

	return args;
}

static AST_Expression * parser_parse_expression_elementary(Parser * parser) {
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		Token const * identifier = parser_advance(parser);

		return ast_make_expr_var(parser->current_line, identifier->value_str);
	} else if (
		parser_match(parser, TOKEN_LITERAL_INT)    ||
		parser_match(parser, TOKEN_LITERAL_F32)    ||
		parser_match(parser, TOKEN_LITERAL_F64)    ||
		parser_match(parser, TOKEN_LITERAL_BOOL)   ||
		parser_match(parser, TOKEN_LITERAL_CHAR)   ||
		parser_match(parser, TOKEN_LITERAL_STRING) ||
		parser_match(parser, TOKEN_KEYWORD_NULL)
	) {
		return ast_make_expr_const(parser->current_line, parser_advance(parser));
	} else if (parser_match(parser, TOKEN_KEYWORD_CAST)) {
		parser_advance(parser);

		parser_expect(parser, TOKEN_PARENTESES_OPEN);
		Type const * type = parser_parse_type(parser);
		parser_expect(parser, TOKEN_PARENTESES_CLOSE);

		AST_Expression * expr = parser_parse_expression(parser);
		
		return ast_make_expr_cast(parser->current_line, type, expr);
	} else if (parser_match(parser, TOKEN_KEYWORD_SIZEOF)) {
		parser_advance(parser);

		parser_expect(parser, TOKEN_PARENTESES_OPEN);
		Type const * type = parser_parse_type(parser);
		parser_expect(parser, TOKEN_PARENTESES_CLOSE);

		return ast_make_expr_sizeof(parser->current_line, type);
	} else if (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
		parser_advance(parser);

		AST_Expression * expr = parser_parse_expression(parser);

		parser_expect(parser, TOKEN_PARENTESES_CLOSE);

		return expr;
	} else {
		parser_error(parser);
	}
}

static AST_Expression * parser_parse_expression_dot_or_array_access(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_elementary(parser);
	
	// Left Associative
	while (true) {
		if (parser_match(parser, TOKEN_DOT)) {
			parser_advance(parser);

			char const * member_name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;
			lhs = ast_make_expr_struct_member(parser->current_line, lhs, member_name);
		} else if (parser_match(parser, TOKEN_SQUARE_BRACES_OPEN)) {
			parser_advance(parser);

			AST_Expression * expr_index = parser_parse_expression(parser);
			parser_expect(parser, TOKEN_SQUARE_BRACES_CLOSE);

			lhs = ast_make_expr_array_access(parser->current_line, lhs, expr_index);
		} else {
			break;
		}		
	}

	return lhs;
}

static AST_Expression * parser_parse_exprssion_call(Parser * parser) {
	AST_Expression * expr = parser_parse_expression_dot_or_array_access(parser);

	while (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
		int            arg_count;
		AST_Call_Arg * args = parser_parse_call_args(parser, &arg_count);

		expr = ast_make_expr_call(parser->current_line, expr, arg_count, args);
	}

	return expr;
}

static AST_Expression * parser_parse_expression_postfix(Parser * parser) {
	AST_Expression * operand = parser_parse_exprssion_call(parser);

	if (parser_match(parser, TOKEN_OPERATOR_INC) || parser_match(parser, TOKEN_OPERATOR_DEC)) {
		Token const * operator = parser_advance(parser);
		return ast_make_expr_op_post(parser->current_line, operator->type, operand);
	}

	return operand;
}

static AST_Expression * parser_parse_expression_prefix(Parser * parser) {
	if (parser_match(parser, TOKEN_OPERATOR_INC) ||
		parser_match(parser, TOKEN_OPERATOR_DEC) ||
		parser_match(parser, TOKEN_OPERATOR_PLUS)  ||
		parser_match(parser, TOKEN_OPERATOR_MINUS) ||
		parser_match(parser, TOKEN_OPERATOR_BITWISE_AND) || // address of
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY)    || // dereference
		parser_match(parser, TOKEN_OPERATOR_BITWISE_NOT) ||
		parser_match(parser, TOKEN_OPERATOR_LOGICAL_NOT)
	) {
		Token    const * operator = parser_advance(parser);
		AST_Expression * operand  = parser_parse_expression_prefix(parser);
		return ast_make_expr_op_pre(parser->current_line, operator->type, operand);
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
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_prefix(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
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
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_multiplicative(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
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
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_additive(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
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
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_bitshift(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
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
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_relational(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_bitwise_and(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_equality(parser);

	// Left Associative
	while (parser_match(parser, TOKEN_OPERATOR_BITWISE_AND)) {
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_equality(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_bitwise_xor(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_bitwise_and(parser);

	// Left Associative
	while (parser_match(parser, TOKEN_OPERATOR_BITWISE_XOR)) {
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_bitwise_and(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_bitwise_or(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_bitwise_xor(parser);

	// Left Associative
	while (parser_match(parser, TOKEN_OPERATOR_BITWISE_OR)) {
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_bitwise_xor(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_logical_and(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_bitwise_or(parser);

	// Left Associative
	while (parser_match(parser, TOKEN_OPERATOR_LOGICAL_AND)) {
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_bitwise_or(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_logical_or(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_logical_and(parser);

	// Left Associative
	while (parser_match(parser, TOKEN_OPERATOR_LOGICAL_OR)) {
		Token    const * operator = parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression_logical_and(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, operator->type, lhs, rhs);
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_assign(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_logical_or(parser);

	// Right Associative
	if (parser_match(parser, TOKEN_ASSIGN)) {
		parser_advance(parser);
		AST_Expression * rhs = parser_parse_expression(parser);

		lhs = ast_make_expr_op_bin(parser->current_line, OPERATOR_BIN_ASSIGN, lhs, rhs);
	} else if (
		parser_match(parser, TOKEN_ASSIGN_PLUS) ||
		parser_match(parser, TOKEN_ASSIGN_MINUS) ||
		parser_match(parser, TOKEN_ASSIGN_MULTIPLY) ||
		parser_match(parser, TOKEN_ASSIGN_DIVIDE) ||
		parser_match(parser, TOKEN_ASSIGN_MODULO) ||
		parser_match(parser, TOKEN_ASSIGN_SHIFT_LEFT) ||
		parser_match(parser, TOKEN_ASSIGN_SHIFT_RIGHT) ||
		parser_match(parser, TOKEN_ASSIGN_BITWISE_AND) ||
		parser_match(parser, TOKEN_ASSIGN_BITWISE_XOR) ||
		parser_match(parser, TOKEN_ASSIGN_BITWISE_OR)
	) {
		Token const * token = parser_advance(parser);
		Operator_Bin operator;
		switch (token->type) {
			case TOKEN_ASSIGN_PLUS:        operator = OPERATOR_BIN_PLUS; break;
			case TOKEN_ASSIGN_MINUS:       operator = OPERATOR_BIN_MINUS; break;
			case TOKEN_ASSIGN_MULTIPLY:    operator = OPERATOR_BIN_MULTIPLY; break;
			case TOKEN_ASSIGN_DIVIDE:      operator = OPERATOR_BIN_DIVIDE; break;
			case TOKEN_ASSIGN_MODULO:      operator = OPERATOR_BIN_MODULO; break;
			case TOKEN_ASSIGN_SHIFT_LEFT:  operator = OPERATOR_BIN_SHIFT_LEFT; break;
			case TOKEN_ASSIGN_SHIFT_RIGHT: operator = OPERATOR_BIN_SHIFT_RIGHT; break;
			case TOKEN_ASSIGN_BITWISE_AND: operator = OPERATOR_BIN_BITWISE_AND; break;
			case TOKEN_ASSIGN_BITWISE_XOR: operator = OPERATOR_BIN_BITWISE_XOR; break;
			case TOKEN_ASSIGN_BITWISE_OR:  operator = OPERATOR_BIN_BITWISE_OR; break;

			default: error_internal();
		}

		AST_Expression * rhs = parser_parse_expression(parser);
		lhs = ast_make_expr_op_bin(parser->current_line, OPERATOR_BIN_ASSIGN, lhs, ast_make_expr_op_bin(parser->current_line, operator, lhs, rhs));
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
	AST_Expression * expr = parser_parse_expression(parser);
	parser_expect(parser, TOKEN_SEMICOLON);

	return ast_make_stat_expr(parser->current_line, expr);
}

static AST_Statement * parser_parse_statement_def_var(Parser * parser) {
	char const * var_name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;
	parser_expect(parser, TOKEN_COLON);

	Type const * type = NULL;
	if (parser_match_type(parser)) {
		type = parser_parse_type(parser);
	}

	AST_Expression * expr_assign = NULL;
	if (parser_match(parser, TOKEN_ASSIGN)) {
		parser_advance(parser);

		// Construct assignment expression, as syntactic sugar
		AST_Expression * lhs = ast_make_expr_var(parser->current_line, var_name);
		AST_Expression * rhs = parser_parse_expression(parser);
		
		expr_assign = ast_make_expr_op_bin(parser->current_line, OPERATOR_BIN_ASSIGN, lhs, rhs);
	}

	parser_expect(parser, TOKEN_SEMICOLON);

	if (type == NULL) {
		if (expr_assign == NULL) {
			error(ERROR_PARSER, "Variable definition missing both type and expression!\n");
		}
		type = type_infer(expr_assign->expr_op_bin.expr_right, parser->current_scope);
	}

	scope_add_var(parser->current_scope, var_name, type);

	return ast_make_stat_def_var(parser->current_line, var_name, type, expr_assign);
}

static void parser_parse_def_arg(Parser * parser,  AST_Def_Arg * arg) {
	arg->name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;
	parser_expect(parser, TOKEN_COLON);
	arg->type = parser_parse_type(parser);
}

static AST_Def_Arg * parser_parse_def_args(Parser * parser, int * arg_count) {
	*arg_count = 0;

	int arg_capacity = 16;
	AST_Def_Arg * args = mem_alloc(arg_capacity * sizeof(AST_Def_Arg));

	parser_expect(parser, TOKEN_PARENTESES_OPEN);
	
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		parser_parse_def_arg(parser, args + (*arg_count)++);
	}

	while (parser_match(parser, TOKEN_COMMA)) {
		parser_advance(parser);

		if (*arg_count == arg_capacity) {
			arg_capacity *= 2;
			args = mem_realloc(args, arg_capacity * sizeof (AST_Def_Arg));
		}

		parser_parse_def_arg(parser, args + (*arg_count)++);
	}
		
	parser_expect(parser, TOKEN_PARENTESES_CLOSE);

	return args;
}

static AST_Statement * parser_parse_statement_def_func(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_FUNC);

	char const * func_name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;

	Variable_Buffer * buffer_args = make_variable_buffer(func_name);
	Variable_Buffer * buffer_vars = make_variable_buffer(func_name);

	Scope * scope_args = make_scope(buffer_args);
	scope_args->prev = parser->current_scope;
	scope_args->variable_buffer->size = 16; // Bias by 16 to account for return address and RBP on stack

	Variable_Buffer * prev_variable_buffer = parser->current_variable_buffer;

	Function_Def * function_def = scope_add_function_def(parser->current_scope);
	function_def->name = func_name;
	function_def->args = parser_parse_def_args(parser, &function_def->arg_count);

	parser->current_variable_buffer = buffer_args;
	parser->current_scope           = scope_args;

	if (parser_match(parser, TOKEN_ARROW)) {
		parser_advance(parser);

		function_def->return_type = parser_parse_type(parser);
	} else {
		function_def->return_type = make_type_void();
	}

	parser->current_variable_buffer = buffer_vars;

	for (int i = 0; i < function_def->arg_count; i++) {
		AST_Def_Arg * arg = &function_def->args[i];
		scope_add_arg(scope_args, arg->name, arg->type);
	}
	
	Type const ** type_args = NULL;
	if (function_def->arg_count > 0) {
		type_args = mem_alloc(function_def->arg_count * sizeof(Type *));

		for (int i = 0; i < function_def->arg_count; i++) {
			type_args[i] = function_def->args[i].type;
		}
	}

	scope_add_var(parser->current_scope->prev, func_name, make_type_function(type_args, function_def->arg_count, function_def->return_type));

	AST_Statement * body = parser_parse_statement_block(parser);

	parser->current_variable_buffer = prev_variable_buffer; // Pop Variable Buffer
	parser->current_scope = parser->current_scope->prev;
	
	return ast_make_stat_def_func(parser->current_line, function_def, buffer_args, buffer_vars, scope_args, body);
}

static AST_Statement * parser_parse_statement_def_struct(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_STRUCT);

	char const * name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;

	Variable_Buffer * members = make_variable_buffer(name);
	Scope           * member_scope = make_scope(members);
	member_scope->prev = parser->current_scope;

	parser_expect(parser, TOKEN_BRACES_OPEN);

	while (parser_match(parser, TOKEN_IDENTIFIER)) {
		char const * member_name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;
		parser_expect(parser, TOKEN_COLON);

		Type const * member_type = parser_parse_type(parser);
		parser_expect(parser, TOKEN_SEMICOLON);

		scope_add_var(member_scope, member_name, member_type);
	}
	
	Struct_Def * struct_def  = scope_add_struct_def(parser->current_scope);
	struct_def->name         = name;
	struct_def->member_scope = member_scope;
	
	align(&struct_def->member_scope->variable_buffer->size, struct_def->member_scope->variable_buffer->align);

	parser_expect(parser, TOKEN_BRACES_CLOSE);

	return NULL;
}

static AST_Statement * parser_parse_statement_extern(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_EXTERN);

	Function_Def * function_def = scope_add_function_def(parser->current_scope);
	function_def->name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;
	function_def->args = parser_parse_def_args(parser, &function_def->arg_count);
	
	if (parser_match(parser, TOKEN_ARROW)) {
		parser_advance(parser);

		function_def->return_type = parser_parse_type(parser);
	} else {
		function_def->return_type = make_type_void();
	}

	parser_expect(parser, TOKEN_SEMICOLON);
	
	Type const ** type_args = NULL;
	if (function_def->arg_count > 0) {
		type_args = mem_alloc(function_def->arg_count * sizeof(Type *));

		for (int i = 0; i < function_def->arg_count; i++) {
			type_args[i] = function_def->args[i].type;
		}
	}

	scope_add_var(parser->current_scope, function_def->name, make_type_function(type_args, function_def->arg_count, function_def->return_type));

	return ast_make_stat_extern(parser->current_line, function_def);
}

static AST_Statement * parser_parse_statement_export(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_EXPORT);

	char const * name = parser_expect(parser, TOKEN_IDENTIFIER)->value_str;

	parser_expect(parser, TOKEN_SEMICOLON);

	return ast_make_stat_export(parser->current_line, name);
}

static AST_Statement * parser_parse_statement_if(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_IF);
	parser_expect(parser, TOKEN_PARENTESES_OPEN);

	AST_Expression * condition = parser_parse_expression(parser);

	parser_expect(parser, TOKEN_PARENTESES_CLOSE);

	AST_Statement * case_true  = parser_parse_statement(parser);
	AST_Statement * case_false = NULL;

	if (parser_match(parser, TOKEN_KEYWORD_ELSE)) {
		parser_advance(parser);

		case_false = parser_parse_statement(parser);
	}

	return ast_make_stat_if(parser->current_line, condition, case_true, case_false);
}

static AST_Statement * parser_parse_statement_while(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_WHILE);

	parser_expect(parser, TOKEN_PARENTESES_OPEN);
	AST_Expression * condition = parser_parse_expression(parser);
	parser_expect(parser, TOKEN_PARENTESES_CLOSE);

	AST_Statement * body = parser_parse_statement(parser);

	return ast_make_stat_while(parser->current_line, condition, body);
}

static AST_Statement * parser_parse_statement_break(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_BREAK);
	parser_expect(parser, TOKEN_SEMICOLON);

	return ast_make_stat_break(parser->current_line);
}

static AST_Statement * parser_parse_statement_continue(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_CONTINUE);
	parser_expect(parser, TOKEN_SEMICOLON);

	return ast_make_stat_continue(parser->current_line);
}

static AST_Statement * parser_parse_statement_return(Parser * parser) {
	parser_expect(parser, TOKEN_KEYWORD_RETURN);

	AST_Expression * expr = NULL;

	if (parser_match_expression(parser)) {
		expr = parser_parse_expression(parser);
	}

	parser_expect(parser, TOKEN_SEMICOLON);

	return ast_make_stat_return(parser->current_line, expr);
}

static AST_Statement * parser_parse_statement_block(Parser * parser) {
	parser_expect(parser, TOKEN_BRACES_OPEN);

	Scope * scope = make_scope(parser->current_variable_buffer);
	parser_push_scope(parser, scope);

	AST_Statement * stat = NULL;
	if (parser_match_statement(parser)) {
		stat = parser_parse_statements(parser);
	}
	
	parser_expect(parser, TOKEN_BRACES_CLOSE);

	parser_pop_scope(parser);

	return ast_make_stat_block(parser->current_line, scope, stat);
}

static AST_Statement * parser_parse_statement(Parser * parser) {
	if (parser_match_statement_def_var(parser)) {
		return parser_parse_statement_def_var(parser);
	} else if (parser_match_statement_def_func(parser)) {
		return parser_parse_statement_def_func(parser);
	} else if (parser_match_statement_def_struct(parser)) {
		return parser_parse_statement_def_struct(parser);
	} else if (parser_match_expression(parser)) {
		return parser_parse_statement_expr(parser);
	} else  if (parser_match_statement_extern(parser)) {
		return parser_parse_statement_extern(parser);
	} else if (parser_match_statement_export(parser)) {
		return parser_parse_statement_export(parser);
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
	AST_Statement * stat_head = parser_parse_statement(parser);

	if (parser_match_statement(parser)) {
		AST_Statement * stat_cons = parser_parse_statements(parser);
		
		return ast_make_stat_stats(parser->current_line, stat_head, stat_cons);
	}

	return stat_head;
}

AST_Statement * parser_parse_program(Parser * parser) {
	Variable_Buffer * globals      = make_variable_buffer(NULL);
	Scope           * global_scope = make_scope(globals);
	
	parser->current_variable_buffer = globals;
	parser->current_scope           = global_scope;

	AST_Statement * stat = parser_parse_statements(parser);

	parser_expect(parser, TOKEN_EOF);

	return ast_make_stat_program(parser->current_line, globals, global_scope, stat);
}

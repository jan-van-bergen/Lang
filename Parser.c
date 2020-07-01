#include "Parser.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

void parser_init(Parser * parser, Token const * tokens, int token_count) {
	parser->token_count = token_count;
	parser->tokens      = tokens;

	parser->index = 0;

	parser->current_variable_buffer = NULL;
	parser->current_scope         = NULL;
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
		parser_match(parser, TOKEN_KEYWORD_CAST)   ||
		parser_match(parser, TOKEN_KEYWORD_SIZEOF) ||
		parser_match(parser, TOKEN_OPERATOR_PLUS)        || // Unary operations
		parser_match(parser, TOKEN_OPERATOR_MINUS)       ||
		parser_match(parser, TOKEN_OPERATOR_INC)         ||
		parser_match(parser, TOKEN_OPERATOR_DEC)         ||
		parser_match(parser, TOKEN_OPERATOR_BITWISE_AND) ||
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY)    ||
		parser_match(parser, TOKEN_OPERATOR_LOGICAL_NOT) ||
		parser_match(parser, TOKEN_LITERAL_INT)    || // Constant values
		parser_match(parser, TOKEN_LITERAL_BOOL)   ||
		parser_match(parser, TOKEN_LITERAL_STRING) ||
		parser_match(parser, TOKEN_PARENTESES_OPEN); // Subexpression
}

static bool parser_match_statement_expr(Parser const * parser) {
	return parser_match_expression(parser);
}

static bool parser_match_statement_def_var(Parser const * parser) {
	return parser_match(parser, TOKEN_KEYWORD_LET);
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

static Type * parser_parse_type(Parser * parser) {
	Type * type = malloc(sizeof(Type));
	type->ptr = NULL;

	char const * identifier = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;

	if (strcmp(identifier, "void") == 0) {
		type->type = TYPE_VOID;
	} else if (strcmp(identifier, "i8") == 0) {
		type->type = TYPE_I8;
	} else if (strcmp(identifier, "i16") == 0) {
		type->type = TYPE_I16;
	} else if (strcmp(identifier, "i32") == 0 || strcmp(identifier, "int") == 0) {
		type->type = TYPE_I32;
	} else if (strcmp(identifier, "i64") == 0) {
		type->type = TYPE_I64;
	} else if (strcmp(identifier, "u8") == 0 || strcmp(identifier, "char") == 0) {
		type->type = TYPE_U8;
	} else if (strcmp(identifier, "u16") == 0) {
		type->type = TYPE_U16;
	} else if (strcmp(identifier, "u32") == 0) {
		type->type = TYPE_U32;
	} else if (strcmp(identifier, "u64") == 0) {
		type->type = TYPE_U64;
	} else if (strcmp(identifier, "bool") == 0) {
		type->type = TYPE_BOOL;
	} else {
		type->type        = TYPE_STRUCT;
		type->struct_name = identifier;
	}

	while (parser_match(parser, TOKEN_OPERATOR_MULTIPLY)) {
		parser_advance(parser);

		Type * type_ptr = malloc(sizeof(Type));
		type_ptr->type = TYPE_POINTER;
		type_ptr->ptr = type;

		type = type_ptr;
	}

	return type;
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
			expr->expr_call.function_name = identifier->value_str;
			expr->expr_call.args = parser_parse_call_args(parser);

			if (expr->expr_call.args == NULL) {
				expr->height = 0;
			} else {
				expr->height = expr->expr_call.args->height;
			}

			return expr;
		} else {
			AST_Expression * expr = malloc(sizeof(AST_Expression));

			expr->type = AST_EXPRESSION_VAR;
			expr->height = 0;
			expr->expr_var.name = identifier->value_str;

			return expr;
		}
	} else if (
		parser_match(parser, TOKEN_LITERAL_INT)  ||
		parser_match(parser, TOKEN_LITERAL_BOOL) ||
		parser_match(parser, TOKEN_LITERAL_STRING)
	) {
		AST_Expression * expr = malloc(sizeof(AST_Expression));

		expr->type = AST_EXPRESSION_CONST;
		expr->height = 0;
		expr->expr_const.token = *parser_advance(parser);

		return expr;
	} else if (parser_match(parser, TOKEN_KEYWORD_CAST)) {
		parser_advance(parser);

		AST_Expression * expr = malloc(sizeof(AST_Expression));
		expr->type = AST_EXPRESSION_CAST;

		parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);
		expr->expr_cast.new_type = parser_parse_type(parser);
		parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

		expr->expr_cast.expr = parser_parse_expression(parser);

		return expr;
	} else if (parser_match(parser, TOKEN_KEYWORD_SIZEOF)) {
		parser_advance(parser);

		AST_Expression * expr = malloc(sizeof(AST_Expression));
		expr->type = AST_EXPRESSION_SIZEOF;

		parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);
		expr->expr_sizeof.type = parser_parse_type(parser);
		parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

		return expr;
	} else if (parser_match(parser, TOKEN_PARENTESES_OPEN)) {
		parser_advance(parser);

		AST_Expression * expr = parser_parse_expression(parser);

		parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

		return expr;
	} else {
		parser_error(parser);
	}
}

static AST_Expression * parser_parse_expression_dot(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_elementary(parser);
	
	// Left Associative
	while (parser_match(parser, TOKEN_DOT)) {
		parser_advance(parser);

		AST_Expression * expr = malloc(sizeof(AST_Expression));
		expr->type = AST_EXPRESSION_STRUCT_MEMBER;

		expr->expr_struct_member.expr = lhs;
		expr->expr_struct_member.member_name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
		
		expr->height = expr->expr_struct_member.expr->height + 1;

		lhs = expr;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_postfix(Parser * parser) {
	AST_Expression * operand =  parser_parse_expression_dot(parser);

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
		parser_match(parser, TOKEN_OPERATOR_MULTIPLY) ||    // dereference
		parser_match(parser, TOKEN_OPERATOR_LOGICAL_NOT)
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

static AST_Expression * parser_parse_expression_logical_and(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_equality(parser);

	// Left Associative
	while (parser_match(parser, TOKEN_OPERATOR_LOGICAL_AND)) {
		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_equality(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_logical_or(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_logical_and(parser);

	// Left Associative
	while (parser_match(parser, TOKEN_OPERATOR_LOGICAL_OR)) {
		AST_Expression * expression = malloc(sizeof(AST_Expression));
		expression->type = AST_EXPRESSION_OPERATOR_BIN;

		expression->expr_op_bin.token = *parser_advance(parser);
		expression->expr_op_bin.expr_left  = lhs;
		expression->expr_op_bin.expr_right = parser_parse_expression_logical_and(parser);

		expression->height = max(
			expression->expr_op_bin.expr_left->height,
			expression->expr_op_bin.expr_right->height
		) + 1;

		lhs = expression;
	}

	return lhs;
}

static AST_Expression * parser_parse_expression_assign(Parser * parser) {
	AST_Expression * lhs = parser_parse_expression_logical_or(parser);

	// Right Associative
	if (parser_match(parser, TOKEN_ASSIGN)) {
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

static AST_Statement * parser_parse_statement_def_var(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_KEYWORD_LET);

	AST_Statement * def = malloc(sizeof(AST_Statement));
	def->type = AST_STATEMENT_DEF_VAR;

	char const * var_name =  parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;

	def->stat_def_var.name = var_name;
	parser_match_and_advance(parser, TOKEN_COLON);
	def->stat_def_var.type = parser_parse_type(parser);

	if (parser_match(parser, TOKEN_ASSIGN)) {
		parser_advance(parser);

		AST_Expression * lhs = malloc(sizeof(AST_Expression));
		lhs->type = AST_EXPRESSION_VAR;
		lhs->expr_var.name = var_name;

		AST_Expression * rhs = parser_parse_expression(parser);
		
		AST_Expression * assign = malloc(sizeof(AST_Expression));
		assign->type = AST_EXPRESSION_OPERATOR_BIN;
		assign->expr_op_bin.token.type = TOKEN_ASSIGN;
		assign->expr_op_bin.expr_left  = lhs;
		assign->expr_op_bin.expr_right = rhs;

		def->stat_def_var.assign = assign;
	} else {
		def->stat_def_var.assign = NULL;
	}

	parser_match_and_advance(parser, TOKEN_SEMICOLON);

	scope_add_var(parser->current_scope, var_name, def->stat_def_var.type);

	return def;
}

static AST_Def_Arg * parser_parse_def_arg(Parser * parser, int * arg_count) {
	AST_Def_Arg * arg = malloc(sizeof(AST_Def_Arg));

	arg->name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	parser_match_and_advance(parser, TOKEN_COLON);
	arg->type = parser_parse_type(parser);
	arg->next = NULL;
	
	(*arg_count)++;

	return arg;
}

static AST_Def_Arg * parser_parse_def_args(Parser * parser, int * arg_count) {
	AST_Def_Arg * args_head = NULL;

	*arg_count = 0;

	parser_match_and_advance(parser, TOKEN_PARENTESES_OPEN);
	
	if (parser_match(parser, TOKEN_IDENTIFIER)) {
		args_head = parser_parse_def_arg(parser, arg_count);
	}
	
	AST_Def_Arg * args_curr = args_head;

	while (parser_match(parser, TOKEN_COMMA)) {
		parser_advance(parser);

		AST_Def_Arg * arg = parser_parse_def_arg(parser, arg_count);
		args_curr->next = arg;
		args_curr       = arg;
	}
		
	parser_match_and_advance(parser, TOKEN_PARENTESES_CLOSE);

	return args_head;
}

static AST_Statement * parser_parse_statement_def_func(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_KEYWORD_FUNC);

	char const * func_name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;

	AST_Statement * func = malloc(sizeof(AST_Statement));
	func->type = AST_STATEMENT_DEF_FUNC;

	func->stat_def_func.buffer_args = make_variable_buffer(func_name);
	func->stat_def_func.buffer_vars = make_variable_buffer(func_name);

	func->stat_def_func.scope_args = make_scope(func->stat_def_func.buffer_args);
	func->stat_def_func.scope_args->prev = parser->current_scope;
	func->stat_def_func.scope_args->variable_buffer->size = 16; // Bias by 16 to account for return address and RBP on stack

	Variable_Buffer * prev_variable_buffer = parser->current_variable_buffer;

	func->stat_def_func.function_def = scope_add_function_def(parser->current_scope);

	parser->current_variable_buffer = func->stat_def_func.buffer_args;
	parser->current_scope           = func->stat_def_func.scope_args;

	func->stat_def_func.function_def->name = func_name;
	func->stat_def_func.function_def->args = parser_parse_def_args(parser, &func->stat_def_func.function_def->arg_count);

	if (parser_match(parser, TOKEN_ARROW)) {
		parser_advance(parser);

		func->stat_def_func.function_def->return_type = parser_parse_type(parser);
	} else {
		func->stat_def_func.function_def->return_type = make_type_void();
	}

	parser->current_variable_buffer = func->stat_def_func.buffer_vars;

	func->stat_def_func.body = parser_parse_statement_block(parser);
	
	AST_Def_Arg * arg = func->stat_def_func.function_def->args;
	while (arg) {
		scope_add_arg(func->stat_def_func.scope_args, arg->name, arg->type);
		
		arg = arg->next;
	}

	parser->current_variable_buffer = prev_variable_buffer; // Pop Variable Buffer
	parser->current_scope = parser->current_scope->prev;

	return func;
}

static AST_Statement * parser_parse_statement_def_struct(Parser * parser) {
	parser_match_and_advance(parser, TOKEN_KEYWORD_STRUCT);

	char const * name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;

	Struct_Def * struct_def = scope_add_struct_def(parser->current_scope);
	struct_def->name = name;
	struct_def->members = make_variable_buffer(name);
	struct_def->member_scope = make_scope(struct_def->members);
	struct_def->member_scope->prev = parser->current_scope;
	
	parser_match_and_advance(parser, TOKEN_BRACES_OPEN);

	while (parser_match(parser, TOKEN_IDENTIFIER)) {
		char const * member_name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;

		parser_match_and_advance(parser, TOKEN_COLON);

		Type * member_type = parser_parse_type(parser);

		parser_match_and_advance(parser, TOKEN_SEMICOLON);

		scope_add_var(struct_def->member_scope, member_name, member_type);
	}
	
	align(&struct_def->members->size, struct_def->members->align);

	parser_match_and_advance(parser, TOKEN_BRACES_CLOSE);

	return NULL;
}

static AST_Statement * parser_parse_statement_extern(Parser * parser) {
	AST_Statement * decl_extern = malloc(sizeof(AST_Statement));
	decl_extern->type = AST_STATEMENT_EXTERN;

	parser_match_and_advance(parser, TOKEN_KEYWORD_EXTERN);
	decl_extern->stat_extern.function_def = scope_add_function_def(parser->current_scope);
	decl_extern->stat_extern.function_def->name = parser_match_and_advance(parser, TOKEN_IDENTIFIER)->value_str;
	decl_extern->stat_extern.function_def->args = parser_parse_def_args(parser, &decl_extern->stat_extern.function_def->arg_count);
	
	if (parser_match(parser, TOKEN_ARROW)) {
		parser_advance(parser);

		decl_extern->stat_extern.function_def->return_type = parser_parse_type(parser);
	} else {
		decl_extern->stat_extern.function_def->return_type = make_type_void();
	}

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

	AST_Statement * block = malloc(sizeof(AST_Statement));
	block->type = AST_STATEMENT_BLOCK;
	block->stat_block.scope = make_scope(parser->current_variable_buffer);

	// Push Scope
	block->stat_block.scope->prev = parser->current_scope;
	parser->current_scope = block->stat_block.scope;

	if (parser_match_statement(parser)) {
		block->stat_block.stat = parser_parse_statements(parser);
	} else {
		block->stat_block.stat = NULL;
	}
	
	// Pop scope
	parser->current_scope = parser->current_scope->prev;

	parser_match_and_advance(parser, TOKEN_BRACES_CLOSE);

	return block;
}

static AST_Statement * parser_parse_statement(Parser * parser) {
	if (parser_match_expression(parser)) {
		return parser_parse_statement_expr(parser);
	} else if (parser_match_statement_def_var(parser)) {
		return parser_parse_statement_def_var(parser);
	} else if (parser_match_statement_def_func(parser)) {
		return parser_parse_statement_def_func(parser);
	} else if (parser_match_statement_def_struct(parser)) {
		return parser_parse_statement_def_struct(parser);
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
	AST_Statement * program = malloc(sizeof(AST_Statement));
	program->type = AST_STATEMENT_PROGRAM;

	program->stat_program.globals      = make_variable_buffer(NULL);
	program->stat_program.global_scope = make_scope(program->stat_program.globals);
	
	parser->current_variable_buffer = program->stat_program.globals;
	parser->current_scope         = program->stat_program.global_scope;

	program->stat_program.stat = parser_parse_statements(parser);

	parser_match_and_advance(parser, TOKEN_EOF);

	return program;
}

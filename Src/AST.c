#include "AST.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Util.h"
#include "Error.h"

AST_Expression * ast_make_expr_const(Token const * token) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_CONST;
	expr->height = 0;

	expr->expr_const.token = *token;

	return expr;
}

AST_Expression * ast_make_expr_var(char const * name) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_VAR;
	expr->height = 0;

	expr->expr_var.name = name;

	return expr;
}

AST_Expression * ast_make_expr_array_access(AST_Expression * expr_array, AST_Expression * expr_index) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_ARRAY_ACCESS;
	expr->height = expr_index->height + 1;

	expr->expr_array_access.expr_array = expr_array;
	expr->expr_array_access.expr_index = expr_index;
		
	return expr;
}
AST_Expression * ast_make_expr_struct_member(AST_Expression * expr_struct, char  const * member_name) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_STRUCT_MEMBER;
	expr->height = expr_struct->height + 1;

	expr->expr_struct_member.expr = expr_struct;
	expr->expr_struct_member.member_name = member_name;
		
	return expr;
}

AST_Expression * ast_make_expr_cast(Type const * type, AST_Expression * expr_cast) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_CAST;
	expr->height = expr_cast->height + 1;

	expr->expr_cast.new_type = type;
	expr->expr_cast.expr     = expr_cast;

	return expr;
}

AST_Expression * ast_make_expr_sizeof(Type const * type) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_SIZEOF;
	expr->height = 0;

	expr->expr_sizeof.type = type;

	return expr;
}

AST_Expression * ast_make_expr_op_bin(Token const * token, AST_Expression * expr_left, AST_Expression * expr_right) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_OPERATOR_BIN;
	expr->height = MAX(expr_left->height, expr_right->height) + 1;

	expr->expr_op_bin.token      = *token;
	expr->expr_op_bin.expr_left  = expr_left;
	expr->expr_op_bin.expr_right = expr_right;

	return expr;
}

AST_Expression * ast_make_expr_op_pre(Token const * token, AST_Expression * expr_operand) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_OPERATOR_PRE;
	expr->height = expr_operand->height + 1;

	expr->expr_op_pre.token = *token;
	expr->expr_op_pre.expr  = expr_operand;

	return expr;
}

AST_Expression * ast_make_expr_op_post(Token const * token, AST_Expression * expr_operand) {
	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_OPERATOR_POST;
	expr->height = expr_operand->height + 1;

	expr->expr_op_post.token = *token;
	expr->expr_op_post.expr  = expr_operand;

	return expr;
}

AST_Expression * ast_make_expr_call(char const * function_name, int arg_count, AST_Call_Arg * args) {
	int height = 0;
	for (int i = 0; i < arg_count; i++) {
		height = MAX(height, args[i].height);
	}

	AST_Expression * expr = malloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_CALL_FUNC;
	expr->height = height + 1;

	expr->expr_call.function_name = function_name;

	expr->expr_call.arg_count = arg_count;
	expr->expr_call.args      = args;

	return expr;
}


AST_Statement * ast_make_stat_program(Variable_Buffer * globals, Scope * global_scope, AST_Statement * stats) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_PROGRAM;

	stat->stat_program.globals      = globals;
	stat->stat_program.global_scope = global_scope;

	stat->stat_program.stat = stats;

	return stat;
}

AST_Statement * ast_make_stat_stats(AST_Statement * head, AST_Statement * cons) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENTS;

	stat->stat_stats.head = head;
	stat->stat_stats.cons = cons;

	return stat;
}

AST_Statement * ast_make_stat_block(Scope * scope, AST_Statement * stats) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_BLOCK;

	stat->stat_block.scope = scope;
	stat->stat_block.stat  = stats;

	return stat;
}

AST_Statement * ast_make_stat_expr(AST_Expression * expr) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_EXPR;

	stat->stat_expr.expr = expr;

	return stat;
}

AST_Statement * ast_make_stat_def_var(char const * name, Type const * type, AST_Expression * assign) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_DEF_VAR;

	stat->stat_def_var.name   = name;
	stat->stat_def_var.type   = type;
	stat->stat_def_var.assign = assign;

	return stat;
}

AST_Statement * ast_make_stat_def_func(Function_Def * function_def, Variable_Buffer * buffer_args, Variable_Buffer * buffer_vars, Scope * scope_args, AST_Statement * body) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_DEF_FUNC;

	stat->stat_def_func.function_def = function_def;
	stat->stat_def_func.buffer_args  = buffer_args;
	stat->stat_def_func.buffer_vars  = buffer_vars;

	stat->stat_def_func.scope_args = scope_args;

	stat->stat_def_func.body = body;

	return stat;
}

AST_Statement * ast_make_stat_extern(Function_Def * function_def) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_EXTERN;

	stat->stat_def_func.function_def = function_def;

	return stat;
}

AST_Statement * ast_make_stat_if(AST_Expression * condition, AST_Statement * case_true, AST_Statement * case_false) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_IF;

	stat->stat_if.condition  = condition;
	stat->stat_if.case_true  = case_true;
	stat->stat_if.case_false = case_false;

	return stat;
}

AST_Statement * ast_make_stat_while(AST_Expression * condition, AST_Statement * body) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_WHILE;
	
	stat->stat_while.condition = condition;
	stat->stat_while.body      = body;

	return stat;
}

AST_Statement * ast_make_stat_break() {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_BREAK;
	
	return stat;
}

AST_Statement * ast_make_stat_continue() {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_CONTINUE;
	
	return stat;
}

AST_Statement * ast_make_stat_return(AST_Expression * expr) {
	AST_Statement * stat = malloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_RETURN;

	stat->stat_return.expr = expr;

	return stat;
}


#define  SPRINTF(fmt)      (*string_offset += sprintf_s(string + *string_offset, string_size - *string_offset, fmt))
#define VSPRINTF(fmt, ...) (*string_offset += sprintf_s(string + *string_offset, string_size - *string_offset, fmt, __VA_ARGS__))

static void print_indent(int indentation_level, char * string, int * string_offset, int string_size) {
	for (int i = 0; i < indentation_level; i++) {
		SPRINTF("    ");
	}
}

static void print_def_arg(AST_Def_Arg const * arg, char * string, int * string_offset, int string_size) {
	char str_type[128];
	type_to_string(arg->type, str_type, sizeof(str_type));

	VSPRINTF("%s: %s", arg->name, str_type);
}

static void print_expression(AST_Expression const * expr, char * string, int * string_offset, int string_size) {
	switch(expr->type) {
		case AST_EXPRESSION_CONST: {
			char str_token[128];
			token_to_string(&expr->expr_const.token, str_token, sizeof(str_token));
			
			VSPRINTF("%s", str_token);

			break;
		}

		case AST_EXPRESSION_VAR: {
			VSPRINTF("%s", expr->expr_var.name);

			break;
		}

		case AST_EXPRESSION_ARRAY_ACCESS: {
			print_expression(expr->expr_array_access.expr_array, string, string_offset, string_size);
			SPRINTF("[");
			print_expression(expr->expr_array_access.expr_index, string, string_offset, string_size);
			SPRINTF("]");

			break;
		}

		case AST_EXPRESSION_STRUCT_MEMBER: {
			print_expression(expr->expr_struct_member.expr, string, string_offset, string_size);
			VSPRINTF(".%s", expr->expr_struct_member.member_name);

			break;
		}

		case AST_EXPRESSION_CAST: {
			char str_type[128];
			type_to_string(expr->expr_cast.new_type, str_type, sizeof(str_type));

			VSPRINTF("cast(%s) ", str_type);
			print_expression(expr->expr_cast.expr, string, string_offset, string_size);

			break;
		}

		case AST_EXPRESSION_SIZEOF: {
			char str_type[128];
			type_to_string(expr->expr_sizeof.type, str_type, sizeof(str_type));

			VSPRINTF("sizeof(%s)", str_type);

			break;
		}

		case AST_EXPRESSION_OPERATOR_BIN: {
			Precedence precedence_here  = get_precedence(expr);
			Precedence precedence_left  = get_precedence(expr->expr_op_bin.expr_left);
			Precedence precedence_right = get_precedence(expr->expr_op_bin.expr_right);

			bool needs_parentheses_left  = precedence_left  > precedence_here;
			bool needs_parentheses_right = precedence_right > precedence_here;

			if (needs_parentheses_left) SPRINTF("(");
			print_expression(expr->expr_op_bin.expr_left, string, string_offset, string_size);
			if (needs_parentheses_left) SPRINTF(")");

			char token_string[128];
			token_to_string(&expr->expr_op_bin.token, token_string, sizeof(token_string));
			VSPRINTF(" %s ", token_string);
			
			if (needs_parentheses_right) SPRINTF("(");
			print_expression(expr->expr_op_bin.expr_right, string, string_offset, string_size);
			if (needs_parentheses_right) SPRINTF(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_PRE: {
			Precedence precedence_here  = get_precedence(expr);
			Precedence precedence_inner = get_precedence(expr->expr_op_pre.expr);

			bool needs_parentheses = precedence_inner > precedence_here;

			char token_string[128];
			token_to_string(&expr->expr_op_pre.token, token_string, sizeof(token_string));
			VSPRINTF("%s", token_string);
			
			if (needs_parentheses) SPRINTF("(");
			print_expression(expr->expr_op_pre.expr, string, string_offset, string_size);
			if (needs_parentheses) SPRINTF(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_POST: {
			Precedence precedence_here  = get_precedence(expr);
			Precedence precedence_inner = get_precedence(expr->expr_op_pre.expr);

			bool needs_parentheses = precedence_inner > precedence_here;

			if (needs_parentheses) SPRINTF("(");
			print_expression(expr->expr_op_post.expr, string, string_offset, string_size);
			if (needs_parentheses) SPRINTF(")");

			char token_string[128];
			token_to_string(&expr->expr_op_post.token, token_string, sizeof(token_string));
			VSPRINTF("%s", token_string);

			break;
		}

		case AST_EXPRESSION_CALL_FUNC: {
			VSPRINTF("%s(", expr->expr_call.function_name);

			for (int i = 0; i < expr->expr_call.arg_count; i++) {
				print_expression(expr->expr_call.args[i].expr, string, string_offset, string_size);

				if (i != expr->expr_call.arg_count - 1) SPRINTF(", ");
			}

			SPRINTF(")");

			break;
		}

		default: SPRINTF("Unprintable Expression!\n"); return;
	}
}

static void print_statement(AST_Statement const * stat, char * string, int * string_offset, int string_size, int indent) {
	switch (stat->type) {
		case AST_STATEMENT_PROGRAM: {
			print_statement(stat->stat_program.stat, string, string_offset, string_size, indent);

			break;
		}

		case AST_STATEMENTS: {
			if (stat->stat_stats.head) print_statement(stat->stat_stats.head, string, string_offset, string_size, indent);
			if (stat->stat_stats.cons) print_statement(stat->stat_stats.cons, string, string_offset, string_size, indent);

			break;
		}

		case AST_STATEMENT_BLOCK: {
			print_statement(stat->stat_block.stat, string, string_offset, string_size, indent);

			break;
		}

		case AST_STATEMENT_EXPR: {
			print_indent(indent, string, string_offset, string_size);
			print_expression(stat->stat_expr.expr, string, string_offset, string_size);
			SPRINTF(";\n");

			break;
		}

		case AST_STATEMENT_DEF_VAR: {
			char str_type[128];
			type_to_string(stat->stat_def_var.type, str_type, sizeof(str_type));

			print_indent(indent, string, string_offset, string_size);
			VSPRINTF("let %s: %s", stat->stat_def_var.name, str_type);

			if (stat->stat_def_var.assign) {
				SPRINTF("; ");
				print_expression(stat->stat_def_var.assign, string, string_offset, string_size);
			}
			SPRINTF(";\n");

			break;
		}

		case AST_STATEMENT_DEF_FUNC: {
			print_indent(indent, string, string_offset, string_size);
			VSPRINTF("func %s(", stat->stat_def_func.function_def->name);

			for (int i = 0; i < stat->stat_def_func.function_def->arg_count; i++) {
				print_def_arg(&stat->stat_def_func.function_def->args[i], string, string_offset, string_size);
			}

			char str_type[128];
			type_to_string(stat->stat_def_func.function_def->return_type, str_type, sizeof(str_type));

			VSPRINTF(") -> %s {\n", str_type);

			print_statement(stat->stat_def_func.body, string, string_offset, string_size, indent + 1);

			print_indent(indent, string, string_offset, string_size);
			SPRINTF("}\n");

			break;
		}

		case AST_STATEMENT_EXTERN: {
			print_indent(indent, string, string_offset, string_size);
			VSPRINTF("extern %s\n", stat->stat_extern.function_def->name);

			break;
		}

		case AST_STATEMENT_IF: {
			print_indent(indent, string, string_offset, string_size);
			SPRINTF("if (");
			print_expression(stat->stat_if.condition, string, string_offset, string_size);
			SPRINTF(") {\n");
			print_statement(stat->stat_if.case_true, string, string_offset, string_size, indent + 1);

			if (stat->stat_if.case_false) {
				print_indent(indent, string, string_offset, string_size);
				SPRINTF("} else {\n");
				print_statement(stat->stat_if.case_false, string, string_offset, string_size, indent + 1);
			}
		
			print_indent(indent, string, string_offset, string_size);
			SPRINTF("}\n");

			break;
		}

		case AST_STATEMENT_WHILE: {
			print_indent(indent, string, string_offset, string_size);
			SPRINTF("while (");
			print_expression(stat->stat_while.condition, string, string_offset, string_size);
			SPRINTF(") {\n");

			print_statement(stat->stat_while.body, string, string_offset, string_size, indent + 1);

			print_indent(indent, string, string_offset, string_size);
			SPRINTF("}\n");

			break;
		}

		case AST_STATEMENT_RETURN: {
			print_indent(indent, string, string_offset, string_size);
			SPRINTF("return");

			if (stat->stat_return.expr) {
				SPRINTF(" ");
				print_expression(stat->stat_return.expr, string, string_offset, string_size);
			}

			SPRINTF(";\n");

			break;
		}

		case AST_STATEMENT_BREAK: {
			print_indent(indent, string, string_offset, string_size);
			SPRINTF("break;\n");

			break;
		}

		case AST_STATEMENT_CONTINUE: {
			print_indent(indent, string, string_offset, string_size);
			SPRINTF("continue;\n");

			break;
		}

		default: SPRINTF("Unprintable Statement!\n");
	}
}

#undef  SPRINTF
#undef VSPRINTF

void ast_print_expression(AST_Expression const * expr, char * string, int string_size) {
	int string_offset = 0;
	print_expression(expr, string, &string_offset, string_size);
}

void ast_print_statement (AST_Statement  const * stat, char * string, int string_size) {
	int string_offset = 0;
	print_statement(stat, string, &string_offset, string_size, 0);
}

static void ast_free_expression(AST_Expression * expr) {
	if (expr == NULL) return;

	switch (expr->type) {
		case AST_EXPRESSION_CONST: break;
		case AST_EXPRESSION_VAR:   break;

		case AST_EXPRESSION_OPERATOR_BIN: {
			ast_free_expression(expr->expr_op_bin.expr_left);
			ast_free_expression(expr->expr_op_bin.expr_right);

			break;
		}

		case AST_EXPRESSION_OPERATOR_PRE: {
			ast_free_expression(expr->expr_op_pre.expr);

			break;
		}

		case AST_EXPRESSION_OPERATOR_POST: {
			ast_free_expression(expr->expr_op_post.expr);

			break;
		}

		case AST_EXPRESSION_CALL_FUNC: {
			free(expr->expr_call.function_name);

			for (int i = 0; i < expr->expr_call.arg_count; i++) {
				ast_free_expression(expr->expr_call.args[i].expr);
			}
			free(expr->expr_call.args);

			break;
		}
	}
}

void ast_free_statement(AST_Statement * stat) {
	if (stat == NULL) return;

	switch (stat->type) {
		case AST_STATEMENT_PROGRAM: {
			ast_free_statement(stat->stat_program.stat);

			break;
		}

		case AST_STATEMENTS: {
			if (stat->stat_stats.head) ast_free_statement(stat->stat_stats.head);
			if (stat->stat_stats.cons) ast_free_statement(stat->stat_stats.cons);

			break;
		}

		case AST_STATEMENT_BLOCK: {
			free_scope(stat->stat_block.scope);
			ast_free_statement(stat->stat_block.stat);

			break;
		}

		case AST_STATEMENT_EXPR: ast_free_expression(stat->stat_expr.expr); break;

		case AST_STATEMENT_DEF_VAR: {
			free(stat->stat_def_var.name);

			break;
		}

		case AST_STATEMENT_DEF_FUNC: {
			free(stat->stat_def_func.function_def->name);

			for (int i = 0; i < stat->stat_def_func.function_def->arg_count; i++) {
				free(stat->stat_def_func.function_def->args[i].name);
			}
			free(stat->stat_def_func.function_def->args);

			ast_free_statement(stat->stat_def_func.body);

			break;
		}

		case AST_STATEMENT_EXTERN: {
			free(stat->stat_extern.function_def->name);

			break;
		}

		case AST_STATEMENT_IF: {
			ast_free_expression(stat->stat_if.condition);

			ast_free_statement(stat->stat_if.case_true);
			ast_free_statement(stat->stat_if.case_false);

			break;
		}

		case AST_STATEMENT_WHILE: {
			ast_free_expression(stat->stat_while.condition);

			ast_free_statement(stat->stat_while.body);

			break;
		}

		case AST_STATEMENT_BREAK:    break;
		case AST_STATEMENT_CONTINUE: break;

		case AST_STATEMENT_RETURN: {
			ast_free_expression(stat->stat_return.expr);

			break;
		}

		default: exit(ERROR_UNKNOWN);
	}

	free(stat);
}

Precedence get_precedence(AST_Expression const * expr) {
	switch (expr->type) {
		case AST_EXPRESSION_ARRAY_ACCESS: return PRECEDENCE_ARRAY_ACCESS;

		case AST_EXPRESSION_OPERATOR_POST: return PRECEDENCE_UNARY_POST;
		case AST_EXPRESSION_OPERATOR_PRE:  return PRECEDENCE_UNARY_PRE;

		case AST_EXPRESSION_CAST: 
		case AST_EXPRESSION_SIZEOF: return PRECEDENCE_CAST_SIZEOF;

		case AST_EXPRESSION_OPERATOR_BIN: {
			switch (expr->expr_op_bin.token.type) {
				case TOKEN_OPERATOR_MULTIPLY:
				case TOKEN_OPERATOR_DIVIDE:
				case TOKEN_OPERATOR_MODULO: return PRECEDENCE_MULTIPLICATIVE;

				case TOKEN_OPERATOR_PLUS:
				case TOKEN_OPERATOR_MINUS: return PRECEDENCE_ADDITIVE;
					
				case TOKEN_OPERATOR_SHIFT_LEFT:
				case TOKEN_OPERATOR_SHIFT_RIGHT: return PRECEDENCE_SHIFT;
					
				case TOKEN_OPERATOR_LT:
				case TOKEN_OPERATOR_GT:
				case TOKEN_OPERATOR_LT_EQ:
				case TOKEN_OPERATOR_GT_EQ: return PRECEDENCE_RELATIONAL;

				case TOKEN_OPERATOR_EQ:
				case TOKEN_OPERATOR_NE: return PRECEDENCE_EQUALITY;

				case TOKEN_OPERATOR_BITWISE_AND: return PRECEDENCE_BITWISE_AND;
				case TOKEN_OPERATOR_BITWISE_XOR: return PRECEDENCE_BITWISE_XOR;
				case TOKEN_OPERATOR_BITWISE_OR:  return PRECEDENCE_BITWISE_OR;

				case TOKEN_OPERATOR_LOGICAL_AND: return PRECEDENCE_LOGICAL_AND;
				case TOKEN_OPERATOR_LOGICAL_OR:  return PRECEDENCE_LOGICAL_OR;

				defaut: exit(ERROR_UNKNOWN);
			}
		}

		case AST_EXPRESSION_ASSIGN: return PRECEDENCE_ASSIGNMENT;
	}

	return PRECEDENCE_NONE;
}
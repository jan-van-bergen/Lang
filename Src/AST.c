#include "AST.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Util.h"
#include "Error.h"

char const * operator_bin_to_str(Operator_Bin  operator) {
	switch (operator) {
		case OPERATOR_BIN_ASSIGN:      return "=";
		case OPERATOR_BIN_LOGICAL_AND: return "&&";
		case OPERATOR_BIN_LOGICAL_OR:  return "||";
		case OPERATOR_BIN_PLUS:        return "+";
		case OPERATOR_BIN_MINUS:       return "-";
		case OPERATOR_BIN_MULTIPLY:    return "*";
		case OPERATOR_BIN_DIVIDE:      return "/";
		case OPERATOR_BIN_MODULO:      return "%";
		case OPERATOR_BIN_SHIFT_LEFT:  return "<<";
		case OPERATOR_BIN_SHIFT_RIGHT: return ">>";
		case OPERATOR_BIN_LT:          return "<";
		case OPERATOR_BIN_LE:          return "<=";
		case OPERATOR_BIN_GT:          return ">";
		case OPERATOR_BIN_GE:          return ">=";
		case OPERATOR_BIN_EQ:          return "==";
		case OPERATOR_BIN_NE:          return "!=";
		case OPERATOR_BIN_BITWISE_AND: return "&";
		case OPERATOR_BIN_BITWISE_XOR: return "^";
		case OPERATOR_BIN_BITWISE_OR:  return "|";
		default: error_internal();
	}
}

char const * operator_pre_to_str(Operator_Pre  operator) {
	switch (operator) {
		case OPERATOR_PRE_ADDRESS_OF:  return "&";
		case OPERATOR_PRE_DEREF:       return "*";
		case OPERATOR_PRE_INC:         return "++";
		case OPERATOR_PRE_DEC:         return "--";
		case OPERATOR_PRE_PLUS:        return "+";
		case OPERATOR_PRE_MINUS:       return "-";
		case OPERATOR_PRE_LOGICAL_NOT: return "!";
		case OPERATOR_PRE_BITWISE_NOT: return "~";
		default: error_internal();
	}
}

char const * operator_post_to_str(Operator_Post operator) {
	switch (operator) {
		case OPERATOR_POST_INC: return "++";
		case OPERATOR_POST_DEC: return "--";
		default: error_internal();
	}
}

AST_Expression * ast_make_expr_const(int line, Token const * token) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_CONST;
	expr->line = line;
	expr->height = 0;

	expr->expr_const.token = *token;

	return expr;
}

AST_Expression * ast_make_expr_var(int line, char const * name) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_VAR;
	expr->line = line;
	expr->height = 0;

	expr->expr_var.name = name;

	return expr;
}

AST_Expression * ast_make_expr_array_access(int line, AST_Expression * expr_array, AST_Expression * expr_index) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_ARRAY_ACCESS;
	expr->line = line;
	expr->height = expr_index->height + 1;

	expr->expr_array_access.expr_array = expr_array;
	expr->expr_array_access.expr_index = expr_index;
		
	return expr;
}
AST_Expression * ast_make_expr_struct_member(int line, AST_Expression * expr_struct, char  const * member_name) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_STRUCT_MEMBER;
	expr->line = line;
	expr->height = expr_struct->height + 1;

	expr->expr_struct_member.expr = expr_struct;
	expr->expr_struct_member.member_name = member_name;
		
	return expr;
}

AST_Expression * ast_make_expr_cast(int line, Type const * type, AST_Expression * expr_cast) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_CAST;
	expr->line = line;
	expr->height = expr_cast->height + 1;

	expr->expr_cast.new_type = type;
	expr->expr_cast.expr     = expr_cast;

	return expr;
}

AST_Expression * ast_make_expr_sizeof(int line, Type const * type) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_SIZEOF;
	expr->line = line;
	expr->height = 0;

	expr->expr_sizeof.type = type;

	return expr;
}

AST_Expression * ast_make_expr_op_bin(int line, Operator_Bin operator, AST_Expression * expr_left, AST_Expression * expr_right) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_OPERATOR_BIN;
	expr->line = line;
	expr->height = MAX(expr_left->height, expr_right->height) + 1;

	expr->expr_op_bin.operator   = operator;
	expr->expr_op_bin.expr_left  = expr_left;
	expr->expr_op_bin.expr_right = expr_right;

	return expr;
}

AST_Expression * ast_make_expr_op_pre(int line, Operator_Pre operator, AST_Expression * expr_operand) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_OPERATOR_PRE;
	expr->line = line;
	expr->height = expr_operand->height + 1;

	expr->expr_op_bin.operator = operator;
	expr->expr_op_pre.expr     = expr_operand;

	return expr;
}

AST_Expression * ast_make_expr_op_post(int line, Operator_Post operator, AST_Expression * expr_operand) {
	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_OPERATOR_POST;
	expr->line = line;
	expr->height = expr_operand->height + 1;

	expr->expr_op_bin.operator = operator;
	expr->expr_op_post.expr    = expr_operand;

	return expr;
}

AST_Expression * ast_make_expr_call(int line, AST_Expression * expr_function, int arg_count, AST_Call_Arg * args) {
	int height = 0;
	for (int i = 0; i < arg_count; i++) {
		height = MAX(height, args[i].height);
	}

	AST_Expression * expr = mem_alloc(sizeof(AST_Expression));
	expr->type = AST_EXPRESSION_CALL_FUNC;
	expr->line = line;
	expr->height = height + 1;

	expr->expr_call.expr_function = expr_function;

	expr->expr_call.arg_count = arg_count;
	expr->expr_call.args      = args;

	return expr;
}


AST_Statement * ast_make_stat_program(int line, Variable_Buffer * globals, Scope * global_scope, AST_Statement * stats) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_PROGRAM;
	stat->line = line;

	stat->stat_program.globals      = globals;
	stat->stat_program.global_scope = global_scope;

	stat->stat_program.stat = stats;

	return stat;
}

AST_Statement * ast_make_stat_stats(int line, AST_Statement * head, AST_Statement * cons) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENTS;
	stat->line = line;

	stat->stat_stats.head = head;
	stat->stat_stats.cons = cons;

	return stat;
}

AST_Statement * ast_make_stat_block(int line, Scope * scope, AST_Statement * stats) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_BLOCK;
	stat->line = line;

	stat->stat_block.scope = scope;
	stat->stat_block.stat  = stats;

	return stat;
}

AST_Statement * ast_make_stat_expr(int line, AST_Expression * expr) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_EXPR;
	stat->line = line;

	stat->stat_expr.expr = expr;

	return stat;
}

AST_Statement * ast_make_stat_def_var(int line, char const * name, Type const * type, AST_Expression * assign) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_DEF_VAR;
	stat->line = line;

	stat->stat_def_var.name   = name;
	stat->stat_def_var.type   = type;
	stat->stat_def_var.assign = assign;

	return stat;
}

AST_Statement * ast_make_stat_def_func(int line, Function_Def * function_def, Variable_Buffer * buffer_args, Variable_Buffer * buffer_vars, Scope * scope_args, AST_Statement * body) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_DEF_FUNC;
	stat->line = line;

	stat->stat_def_func.function_def = function_def;
	stat->stat_def_func.buffer_args  = buffer_args;
	stat->stat_def_func.buffer_vars  = buffer_vars;

	stat->stat_def_func.scope_args = scope_args;

	stat->stat_def_func.body = body;

	return stat;
}

AST_Statement * ast_make_stat_extern(int line, Function_Def * function_def) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_EXTERN;
	stat->line = line;

	stat->stat_def_func.function_def = function_def;

	return stat;
}

AST_Statement * ast_make_stat_export(int line, char const * name) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_EXPORT;
	stat->line = line;

	stat->stat_export.name = name;

	return stat;
}

AST_Statement * ast_make_stat_if(int line, AST_Expression * condition, AST_Statement * case_true, AST_Statement * case_false) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_IF;
	stat->line = line;

	stat->stat_if.condition  = condition;
	stat->stat_if.case_true  = case_true;
	stat->stat_if.case_false = case_false;

	return stat;
}

AST_Statement * ast_make_stat_while(int line, AST_Expression * condition, AST_Statement * body) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_WHILE;
	stat->line = line;

	stat->stat_while.condition = condition;
	stat->stat_while.body      = body;

	return stat;
}

AST_Statement * ast_make_stat_break(int line) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_BREAK;
	stat->line = line;

	return stat;
}

AST_Statement * ast_make_stat_continue(int line) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_CONTINUE;
	stat->line = line;

	return stat;
}

AST_Statement * ast_make_stat_return(int line, AST_Expression * expr) {
	AST_Statement * stat = mem_alloc(sizeof(AST_Statement));
	stat->type = AST_STATEMENT_RETURN;
	stat->line = line;

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
	char str_type[512];
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

			VSPRINTF(" %s ", operator_bin_to_str(expr->expr_op_bin.operator));
			
			if (needs_parentheses_right) SPRINTF("(");
			print_expression(expr->expr_op_bin.expr_right, string, string_offset, string_size);
			if (needs_parentheses_right) SPRINTF(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_PRE: {
			Precedence precedence_here  = get_precedence(expr);
			Precedence precedence_inner = get_precedence(expr->expr_op_pre.expr);

			bool needs_parentheses = precedence_inner > precedence_here;

			VSPRINTF("%s", operator_pre_to_str(expr->expr_op_pre.operator));
			
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

			VSPRINTF("%s", operator_post_to_str(expr->expr_op_post.operator));

			break;
		}

		case AST_EXPRESSION_CALL_FUNC: {
			print_expression(expr->expr_call.expr_function, string, string_offset, string_size);

			SPRINTF("(");

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
			if (stat->stat_block.stat) print_statement(stat->stat_block.stat, string, string_offset, string_size, indent);

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
			VSPRINTF("%s: %s", stat->stat_def_var.name, str_type);

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

bool ast_is_lvalue(AST_Expression const * expr) {
	return
		expr->type == AST_EXPRESSION_OPERATOR_PRE ||
		expr->type == AST_EXPRESSION_OPERATOR_POST ||
		expr->type == AST_EXPRESSION_VAR ||
		expr->type == AST_EXPRESSION_ARRAY_ACCESS ||
		expr->type == AST_EXPRESSION_STRUCT_MEMBER;
}

bool ast_contains(AST_Expression const * expr, AST_Expression_Type expression_type) {
	if (expr->type == expression_type) return true;

	switch (expr->type) {
		case AST_EXPRESSION_CONST: return false;
		case AST_EXPRESSION_VAR:   return false;
			
		case AST_EXPRESSION_ARRAY_ACCESS: return
			ast_contains(expr->expr_array_access.expr_array, expression_type) ||
			ast_contains(expr->expr_array_access.expr_index, expression_type);
		case AST_EXPRESSION_STRUCT_MEMBER: return ast_contains(expr->expr_struct_member.expr, expression_type);

		case AST_EXPRESSION_CAST:   return ast_contains(expr->expr_cast.expr, expression_type);
		case AST_EXPRESSION_SIZEOF: return false;

		case AST_EXPRESSION_OPERATOR_BIN: return 
			ast_contains(expr->expr_op_bin.expr_left,  expression_type) || 
			ast_contains(expr->expr_op_bin.expr_right, expression_type);
		case AST_EXPRESSION_OPERATOR_PRE:  return ast_contains(expr->expr_op_pre .expr, expression_type);
		case AST_EXPRESSION_OPERATOR_POST: return ast_contains(expr->expr_op_post.expr, expression_type);

		case AST_EXPRESSION_CALL_FUNC: return false;

		default: error_internal();
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
			ast_free_expression(expr->expr_call.expr_function);

			for (int i = 0; i < expr->expr_call.arg_count; i++) {
				ast_free_expression(expr->expr_call.args[i].expr);
			}
			mem_free(expr->expr_call.args);

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
			mem_free(stat->stat_def_var.name);

			break;
		}

		case AST_STATEMENT_DEF_FUNC: {
			mem_free(stat->stat_def_func.function_def->name);

			for (int i = 0; i < stat->stat_def_func.function_def->arg_count; i++) {
				mem_free(stat->stat_def_func.function_def->args[i].name);
			}
			mem_free(stat->stat_def_func.function_def->args);

			ast_free_statement(stat->stat_def_func.body);

			break;
		}

		case AST_STATEMENT_EXTERN: {
			mem_free(stat->stat_extern.function_def->name);

			break;
		}

		case AST_STATEMENT_EXPORT: {
			mem_free(stat->stat_export.name);

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

		default: error_internal();
	}

	mem_free(stat);
}

Precedence get_precedence(AST_Expression const * expr) {
	switch (expr->type) {
		case AST_EXPRESSION_ARRAY_ACCESS: return PRECEDENCE_ARRAY_ACCESS;

		case AST_EXPRESSION_OPERATOR_POST: return PRECEDENCE_UNARY_POST;
		case AST_EXPRESSION_OPERATOR_PRE:  return PRECEDENCE_UNARY_PRE;

		case AST_EXPRESSION_CAST: 
		case AST_EXPRESSION_SIZEOF: return PRECEDENCE_CAST_SIZEOF;

		case AST_EXPRESSION_OPERATOR_BIN: {
			switch (expr->expr_op_bin.operator) {
				case OPERATOR_BIN_ASSIGN: return PRECEDENCE_ASSIGNMENT;

				case OPERATOR_BIN_MULTIPLY:
				case OPERATOR_BIN_DIVIDE:
				case OPERATOR_BIN_MODULO: return PRECEDENCE_MULTIPLICATIVE;

				case OPERATOR_BIN_PLUS:
				case OPERATOR_BIN_MINUS: return PRECEDENCE_ADDITIVE;
					
				case OPERATOR_BIN_SHIFT_LEFT:
				case OPERATOR_BIN_SHIFT_RIGHT: return PRECEDENCE_SHIFT;
					
				case OPERATOR_BIN_LT:
				case OPERATOR_BIN_GT:
				case OPERATOR_BIN_LE:
				case OPERATOR_BIN_GE: return PRECEDENCE_RELATIONAL;

				case OPERATOR_BIN_EQ:
				case OPERATOR_BIN_NE: return PRECEDENCE_EQUALITY;

				case OPERATOR_BIN_BITWISE_AND: return PRECEDENCE_BITWISE_AND;
				case OPERATOR_BIN_BITWISE_XOR: return PRECEDENCE_BITWISE_XOR;
				case OPERATOR_BIN_BITWISE_OR:  return PRECEDENCE_BITWISE_OR;

				case OPERATOR_BIN_LOGICAL_AND: return PRECEDENCE_LOGICAL_AND;
				case OPERATOR_BIN_LOGICAL_OR:  return PRECEDENCE_LOGICAL_OR;

				defaut: error_internal();
			}
		}
	}

	return PRECEDENCE_NONE;
}

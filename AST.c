#include "AST.h"

#include <stdio.h>
#include <stdlib.h>

static void print_indent(int level) {
	for (int i = 0; i < level; i++) {
		printf("    ");
	}
}

static void print_def_arg(AST_Def_Arg const * arg) {
	char str_type[128];
	type_to_string(arg->type, str_type, sizeof(str_type));

	printf("%s: %s", arg->name, str_type);
}

static void print_expression(AST_Expression const * expr) {
	switch(expr->type) {
		case AST_EXPRESSION_CONST: {
			char str_token[128];
			token_to_string(&expr->expr_const.token, str_token, sizeof(str_token));
			printf("%s", str_token);

			break;
		}

		case AST_EXPRESSION_VAR: {
			printf("%s", expr->expr_var.name);

			break;
		}

		case AST_EXPRESSION_STRUCT_MEMBER: {
			print_expression(expr->expr_struct_member.expr);
			printf(".%s", expr->expr_struct_member.member_name);

			break;
		}

		case AST_EXPRESSION_CAST: {
			char str_type[128];
			type_to_string(expr->expr_cast.new_type, str_type, sizeof(str_type));

			printf("cast(%s) ", str_type);
			print_expression(expr->expr_cast.expr);

			break;
		}

		case AST_EXPRESSION_SIZEOF: {
			char str_type[128];
			type_to_string(expr->expr_sizeof.type, str_type, sizeof(str_type));

			printf("sizeof(%s)", str_type);

			break;
		}

		case AST_EXPRESSION_OPERATOR_BIN: {
			printf("(");
			print_expression(expr->expr_op_bin.expr_left);

			char token_string[128];
			token_to_string(&expr->expr_op_bin.token, token_string, sizeof(token_string));
			printf(" %s ", token_string);
			
			print_expression(expr->expr_op_bin.expr_right);
			printf(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_PRE: {
			printf("(");

			char token_string[128];
			token_to_string(&expr->expr_op_pre.token, token_string, sizeof(token_string));
			printf("%s", token_string);
			
			print_expression(expr->expr_op_pre.expr);
			printf(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_POST: {
			printf("(");

			print_expression(expr->expr_op_post.expr);
			
			char token_string[128];
			token_to_string(&expr->expr_op_post.token, token_string, sizeof(token_string));
			printf("%s", token_string);
			printf(")");

			break;
		}

		case AST_EXPRESSION_CALL_FUNC: {
			printf("%s(", expr->expr_call.function_name);

			for (int i = 0; i < expr->expr_call.arg_count; i++) {
				print_expression(expr->expr_call.args[i].expr);
			}

			printf(")");

			break;
		}

		default: printf("Unprintable Expression!\n"); return;
	}
}

static void print_statement(AST_Statement const * stat, int indent) {
	switch (stat->type) {
		case AST_STATEMENT_PROGRAM: {
			print_statement(stat->stat_program.stat, indent);

			break;
		}

		case AST_STATEMENTS: {
			if (stat->stat_stats.head) print_statement(stat->stat_stats.head, indent);
			if (stat->stat_stats.cons) print_statement(stat->stat_stats.cons, indent);

			break;
		}

		case AST_STATEMENT_BLOCK: {
			print_statement(stat->stat_block.stat, indent);

			break;
		}

		case AST_STATEMENT_EXPR: {
			print_indent(indent);
			print_expression(stat->stat_expr.expr);
			printf(";\n");

			break;
		}

		case AST_STATEMENT_DEF_VAR: {
			char str_type[128];
			type_to_string(stat->stat_def_var.type, str_type, sizeof(str_type));

			print_indent(indent);
			printf("let %s: %s", stat->stat_def_var.name, str_type);

			if (stat->stat_def_var.assign) {
				printf("; ");
				print_expression(stat->stat_def_var.assign);
			}
			printf(";\n");

			break;
		}

		case AST_STATEMENT_DEF_FUNC: {
			print_indent(indent);
			printf("func %s(", stat->stat_def_func.function_def->name);

			for (int i = 0; i < stat->stat_def_func.function_def->arg_count; i++) {
				print_def_arg(&stat->stat_def_func.function_def->args[i]);
			}

			char str_type[128];
			type_to_string(stat->stat_def_func.function_def->return_type, str_type, sizeof(str_type));

			printf(") -> %s {\n", str_type);

			print_statement(stat->stat_def_func.body, indent + 1);

			print_indent(indent);
			printf("}\n");

			break;
		}

		case AST_STATEMENT_EXTERN: {
			print_indent(indent);
			printf("extern %s\n", stat->stat_extern.function_def->name);

			break;
		}

		case AST_STATEMENT_IF: {
			print_indent(indent);
			printf("if (");
			print_expression(stat->stat_if.condition);
			printf(") {\n");
			print_statement(stat->stat_if.case_true, indent + 1);

			if (stat->stat_if.case_false) {
				print_indent(indent);
				printf("} else {\n");
				print_statement(stat->stat_if.case_false, indent + 1);
			}
		
			print_indent(indent);
			printf("}\n");

			break;
		}

		case AST_STATEMENT_WHILE: {
			print_indent(indent);
			printf("while (");
			print_expression(stat->stat_while.condition);
			printf(") {\n");

			print_statement(stat->stat_while.body, indent + 1);

			print_indent(indent);
			printf("}\n");

			break;
		}

		case AST_STATEMENT_RETURN: {
			print_indent(indent);
			printf("return");

			if (stat->stat_return.expr) {
				printf(" ");
				print_expression(stat->stat_return.expr);
			}

			printf(";\n");

			break;
		}

		case AST_STATEMENT_BREAK: {
			print_indent(indent);
			printf("break;\n");

			break;
		}

		case AST_STATEMENT_CONTINUE: {
			print_indent(indent);
			printf("continue;\n");

			break;
		}

		default: printf("Unprintable Statement!\n");
	}
}

void ast_pretty_print(AST_Statement const * program) {
	print_statement(program, 0);
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

		default: abort();
	}

	free(stat);
}

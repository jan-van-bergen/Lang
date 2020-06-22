#include "AST.h"

#include <stdio.h>

static void print_indent(int level) {
	for (int i = 0; i < level; i++) {
		printf("    ");
	}
}

static void print_decl_args(AST_Decl_Arg const * arg) {
	if (arg) {
		printf("%s: %s", arg->name, arg->type);

		if (arg->next) {
			printf(", ");
			print_decl_args(arg->next);
		}
	}
}

static void print_expression(AST_Expression const * expr);

static void print_call_args(AST_Call_Arg const * arg) {
	if (arg) {
		print_expression(arg->expr);

		if (arg->next) {
			printf(", ");
			print_call_args(arg->next);
		}
	}
}

static void print_expression(AST_Expression const * expr) {
	switch(expr->type) {
		case AST_EXPRESSION_CONST: {
			char token_string[128];
			token_to_string(&expr->expr_const.token, token_string, sizeof(token_string));
			printf("%s", token_string);

			break;
		}

		case AST_EXPRESSION_VAR: {
			char token_string[128];
			token_to_string(&expr->expr_var.token, token_string, sizeof(token_string));	
			printf("%s", token_string);

			break;
		}

		case AST_EXPRESSION_OPERATOR_BIN: {
			printf("(");
			print_expression(expr->expr_op_bin.expr_left);

			char token_string[128];
			token_to_string(&expr->expr_var.token, token_string, sizeof(token_string));
			printf(" %s ", token_string);
			
			print_expression(expr->expr_op_bin.expr_right);
			printf(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_PRE: {
			printf("(");

			char token_string[128];
			token_to_string(&expr->expr_var.token, token_string, sizeof(token_string));
			printf("%s", token_string);
			
			print_expression(expr->expr_op_pre.expr);
			printf(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_POST: {
			printf("(");

			print_expression(expr->expr_op_pre.expr);
			
			char token_string[128];
			token_to_string(&expr->expr_var.token, token_string, sizeof(token_string));
			printf("%s", token_string);
			printf(")");

			break;
		}

		case AST_EXPRESSION_CALL_FUNC: {
			printf("%s(", expr->expr_call.function);
			print_call_args(expr->expr_call.args);
			printf(")");

			break;
		}

		default: printf("Unprintable Expression!\n"); return;
	}
}

static void print_statement(AST_Statement const * stat, int indent) {
	switch (stat->type) {
		case AST_STATEMENT_NOOP: {
			print_indent(indent);
			printf(";\n");

			break;
		}

		case AST_STATEMENTS: {
			print_statement(stat->stat_stats.head, indent);
			print_statement(stat->stat_stats.cons, indent);

			break;
		}

		case AST_STATEMENT_EXPR: {
			print_indent(indent);
			print_expression(stat->stat_expr.expr, indent);
			printf(";\n");

			break;
		}

		case AST_STATEMENT_DECL_VAR: {
			print_indent(indent);
			printf("let %s: %s;\n", stat->stat_decl_var.name, stat->stat_decl_var.type);

			break;
		}

		case AST_STATEMENT_DECL_FUNC: {
			print_indent(indent);
			printf("func %s(", stat->stat_decl_func.name);

			if (stat->stat_decl_func.args) {
				print_decl_args(stat->stat_decl_func.args, indent);
			}

			printf(") -> %s {\n", stat->stat_decl_func.return_type);

			print_statement(stat->stat_decl_func.body, indent + 1);

			print_indent(indent);
			printf("}\n");

			break;
		}

		case AST_STATEMENT_EXTERN: {
			print_indent(indent);
			printf("extern %s\n", stat->stat_extern.name);

			break;
		}

		case AST_STATEMENT_IF: {
			print_indent(indent);
			printf("if (");
			print_expression(stat->stat_if.condition, indent);
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
			print_expression(stat->stat_while.condition, indent);
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

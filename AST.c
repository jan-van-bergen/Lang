#include "AST.h"

#include <stdio.h>

static void print_newline(int level) {
	printf("\n");

	for (int i = 0; i < level; i++) {
		printf("    ");
	}
}

void ast_debug_expr(AST_Expression const * expr) {
	switch (expr->type) {
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

		default: printf("Unsupported expression type!");
	}
}

void ast_debug_stat(AST_Statement const * stat, int level) {
	switch (stat->type) {
		case AST_STATEMENT_DECL: {
			printf("%s", stat->stat_decl.name);

			if (stat->stat_decl.type) {
				printf(": %s ", stat->stat_decl.type);
			} else {
				printf(" :");
			}

			if (stat->stat_decl.expr) {
				printf("= ");
				ast_debug_expr(stat->stat_decl.expr);
			}

			print_newline(level);

			break;
		}

		case AST_STATEMENT_ASSIGN: {
			printf("%s = ", stat->stat_assign.name);
			ast_debug_expr(stat->stat_assign.expr);
			printf(";");
			print_newline(level);

			break;
		}

		case AST_STATEMENT_IF: {
			printf("if (");
			ast_debug_expr(stat->stat_if.condition);
			printf(") ");
			ast_debug_stat(stat->stat_if.case_true, level);

			if (stat->stat_if.case_false) {
				printf(" else ");
				ast_debug_stat(stat->stat_if.case_false, level);
			}

			break;
		}

		case AST_STATEMENT_BLOCK: {
			printf("{");
			print_newline(level + 1);

			for (int i = 0; i < stat->stat_block.statement_count; i++) {
				ast_debug_stat(&stat->stat_block.statements[i], level + 1);
			}

			print_newline(level);
			printf("}");

			break;
		}

		default: printf("Unsupported statement type!");
	}
}

void ast_debug(AST_Statement const * program) {
	ast_debug_stat(program, 0);
}

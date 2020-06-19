#include "AST.h"

#include <stdio.h>

static void print_indent(int level) {
	for (int i = 0; i < level; i++) {
		printf("    ");
	}
}

static void print_ast(AST_Node const * node, int indent) {
	switch (node->type) {
		case AST_STATEMENTS: {
			print_ast(node->stat_statements.head, indent);
			print_ast(node->stat_statements.cons, indent);

			break;
		}

		case AST_STATEMENT_DECL: {
			print_indent(indent);
			printf("%s", node->stat_decl.name);

			if (node->stat_decl.type) {
				printf(": %s ", node->stat_decl.type, indent);
			} else {
				printf(" :");
			}

			if (node->stat_decl.expr) {
				printf("= ");
				print_ast(node->stat_decl.expr, indent);
			}

			printf("\n");

			break;
		}

		case AST_STATEMENT_ASSIGN: {
			print_indent(indent);
			printf("%s = ", node->stat_assign.name);
			print_ast(node->stat_assign.expr, indent);
			printf(";");
			printf("\n");

			break;
		}

		case AST_STATEMENT_IF: {
			print_indent(indent);
			printf("if (");
			print_ast(node->stat_if.condition, indent);
			printf(") {\n");
			print_ast(node->stat_if.case_true, indent + 1);

			if (node->stat_if.case_false) {
				printf("} else {\n");
				print_ast(node->stat_if.case_false, indent + 1);
			}
		
			print_indent(indent);
			printf("}\n");

			break;
		}

		case AST_EXPRESSION_CONST: {
			char token_string[128];
			token_to_string(&node->expr_const.token, token_string, sizeof(token_string));
			printf("%s", token_string);

			break;
		}

		case AST_EXPRESSION_VAR: {
			char token_string[128];
			token_to_string(&node->expr_var.token, token_string, sizeof(token_string));	
			printf("%s", token_string);

			break;
		}

		case AST_EXPRESSION_OPERATOR_BIN: {
			printf("(");
			print_ast(node->expr_op_bin.expr_left, indent);

			char token_string[128];
			token_to_string(&node->expr_var.token, token_string, sizeof(token_string));
			printf("%s", token_string);
			
			print_ast(node->expr_op_bin.expr_right, indent);
			printf(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_PRE: {
			printf("(");

			char token_string[128];
			token_to_string(&node->expr_var.token, token_string, sizeof(token_string));
			printf("%s", token_string);
			
			print_ast(node->expr_op_pre.expr, indent);
			printf(")");

			break;
		}

		case AST_EXPRESSION_OPERATOR_POST: {
			printf("(");

			print_ast(node->expr_op_pre.expr, indent);
			
			char token_string[128];
			token_to_string(&node->expr_var.token, token_string, sizeof(token_string));
			printf("%s", token_string);
			printf(")");

			break;
		}

		default: printf("Unsupported AST_Node!\n");
	}
}

void ast_pretty_print(AST_Node const * program) {
	print_ast(program, 0);
}

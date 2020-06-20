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

		case AST_STATEMENT_EXPR: {
			print_indent(indent);
			print_ast(node->stat_expr.expr, indent);
			printf(";\n");

			break;
		}

		case AST_STATEMENT_DECL_VAR: {
			print_indent(indent);
			printf("let %s: %s;\n", node->stat_decl.name, node->stat_decl.type);

			break;
		}

		case AST_STATEMENT_DECL_FUNC: {
			print_indent(indent);
			printf("func %s(", node->stat_func.name);

			if (node->stat_func.args) {
				print_ast(node->stat_func.args, indent);
			}

			printf(") -> %s {\n", node->stat_func.return_type);
			
			if (node->stat_func.body) {
				print_ast(node->stat_func.body, indent + 1);
			}

			print_indent(indent);
			printf("}\n");

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

		case AST_EXPRESSION_CALL_FUNC: {
			printf("%s(", node->expr_call.function);
			print_ast(node->expr_call.args, indent);
			printf(")");

			break;
		}

		case AST_DECL_ARGS: {
			printf("%s: %s", node->decl_args.name, node->decl_args.type);

			if (node->decl_args.next) {
				printf(", ");
				print_ast(node->decl_args.next, indent);
			}

			break;
		}

		case AST_CALL_ARGS: {
			print_ast(node->call_args.arg, indent);

			if (node->call_args.next) {
				printf(", ");
				print_ast(node->call_args.next, indent);
			}

			break;
		}

		default: printf("Unprintable AST_Node!\n"); return;
	}
}

void ast_pretty_print(AST_Node const * program) {
	print_ast(program, 0);
}

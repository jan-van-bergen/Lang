//#include "AST.h"
//
//#include <stdio.h>
//
//static void print_indent(int level) {
//	for (int i = 0; i < level; i++) {
//		printf("    ");
//	}
//}
//
//static void print_expr(AST_Expression const * expr) {
//	switch (expr->type) {
//		case AST_EXPRESSION_CONST: {
//			char token_string[128];
//			token_to_string(&expr->expr_const.token, token_string, sizeof(token_string));
//			printf("%s", token_string);
//
//			break;
//		}
//
//		case AST_EXPRESSION_VAR: {
//			char token_string[128];
//			token_to_string(&expr->expr_var.token, token_string, sizeof(token_string));	
//			printf("%s", token_string);
//
//			break;
//		}
//
//		case AST_EXPRESSION_OPERATOR_BIN: {
//			print_expr(expr->expr_op_bin.expr_left);
//
//			char token_string[128];
//			token_to_string(&expr->expr_var.token, token_string, sizeof(token_string));
//			printf("%s", token_string);
//			
//			print_expr(expr->expr_op_bin.expr_right);
//
//			break;
//		}
//
//		default: printf("Unsupported expression type!");
//	}
//}
//
//static void print_stat(AST_Statement const * stat, int level) {
//	switch (stat->type) {
//		case AST_STATEMENT_DECL: {
//			print_indent(level);
//			printf("%s", stat->stat_decl.name);
//
//			if (stat->stat_decl.type) {
//				printf(": %s ", stat->stat_decl.type);
//			} else {
//				printf(" :");
//			}
//
//			if (stat->stat_decl.expr) {
//				printf("= ");
//				print_expr(stat->stat_decl.expr);
//			}
//
//			printf("\n");
//
//			break;
//		}
//
//		case AST_STATEMENT_ASSIGN: {
//			print_indent(level);
//			printf("%s = ", stat->stat_assign.name);
//			print_expr(stat->stat_assign.expr);
//			printf(";");
//			printf("\n");
//
//			break;
//		}
//
//		case AST_STATEMENT_IF: {
//			print_indent(level);
//			printf("if (");
//			print_expr(stat->stat_if.condition);
//			printf(") ");
//			print_stat(stat->stat_if.case_true, level);
//
//			if (stat->stat_if.case_false) {
//				printf(" else ");
//				print_stat(stat->stat_if.case_false, level);
//			}
//		
//			printf("}\n");
//
//			break;
//		}
//
//		case AST_STATEMENT_BLOCK: {
//			printf("{\n");
//			
//			for (int i = 0; i < stat->stat_block.statement_count; i++) {
//				print_stat(&stat->stat_block.statements[i], level + 1);
//			}
//
//			print_indent(level);
//
//			break;
//		}
//
//		default: printf("Unsupported statement type!");
//	}
//}
//
//void ast_debug(AST_Statement const * program) {
//	print_stat(program, 0);
//}

#pragma once
#include "Token.h"

typedef enum AST_Type {
	AST_STATEMENTS,
	AST_STATEMENT_DECL,
	AST_STATEMENT_ASSIGN,
	AST_STATEMENT_FUNC,
	AST_STATEMENT_IF,
	AST_STATEMENT_FOR,

	AST_EXPRESSION_CONST,
	AST_EXPRESSION_VAR,
	AST_EXPRESSION_OPERATOR_BIN,
	AST_EXPRESSION_OPERATOR_PRE,
	AST_EXPRESSION_OPERATOR_POST,

	AST_ARGS
} AST_Type;

typedef struct AST_Node {
	AST_Type type;

	union {
		struct Statements {
			struct AST_Node * head;
			struct AST_Node * cons;
		} stat_statements;

		struct Decl {
			char const * name;
			char const * type;
			struct AST_Node * expr;
		} stat_decl;

		struct Assign {
			char const * name;
			struct AST_Node * expr;
		} stat_assign;

		struct Func {
			char const * name;
			struct AST_Node * args;
			struct AST_Node * body;
		} stat_func;

		struct If {
			struct AST_Node * condition;

			struct AST_Node * case_true;
			struct AST_Node * case_false;
		} stat_if;

		struct For {
			struct AST_Node * expr_init;
			struct AST_Node * expr_condition;
			struct AST_Node * expr_next;

			struct AST_Node * body;
		} stat_for;

		struct Const {
			Token token;
		} expr_const;

		struct Var {
			Token token;
		} expr_var;

		struct Op_Bin {
			Token token;

			struct AST_Node * expr_left;
			struct AST_Node * expr_right;
		} expr_op_bin;

		struct Op_Pre {
			Token token;

			struct AST_Node * expr;
		} expr_op_pre;
		
		struct Op_Post {
			Token token;

			struct AST_Node * expr;
		} expr_op_post;

		struct Args {
			char const * name;
			char const * type;

			struct AST_Node * next;
		} args;
	};
} AST_Node;

void ast_pretty_print(AST_Node const * program);

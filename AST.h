#pragma once
#include "Token.h"

typedef enum AST_Type {
	AST_STATEMENT_NOOP,
	AST_STATEMENTS,

	AST_STATEMENT_EXPR,
	AST_STATEMENT_DECL_VAR,
	AST_STATEMENT_DECL_FUNC,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,

	AST_EXPRESSION_CONST,
	AST_EXPRESSION_VAR,
	AST_EXPRESSION_ASSIGN,
	AST_EXPRESSION_OPERATOR_BIN,
	AST_EXPRESSION_OPERATOR_PRE,
	AST_EXPRESSION_OPERATOR_POST,
	AST_EXPRESSION_CALL_FUNC,

	AST_DECL_ARGS,
	AST_CALL_ARGS
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
		} stat_decl;

		struct Assign {
			char const * name;
			struct AST_Node * expr;
		} stat_assign;

		struct Expr {
			struct AST_Node * expr;
		} stat_expr;

		struct Func {
			char const * name;
			char const * return_type;
			struct AST_Node * args;
			struct AST_Node * body;
		} stat_func;

		struct If {
			struct AST_Node * condition;

			struct AST_Node * case_true;
			struct AST_Node * case_false;
		} stat_if;

		struct While {
			struct AST_Node * condition;
			struct AST_Node * body;
		} stat_while;

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

		struct Call {
			char const * function;

			struct AST_Node * args;
		} expr_call;

		struct DeclArgs {
			char const * name;
			char const * type;

			struct AST_Node * next;
		} decl_args;

		struct CallArgs {
			struct AST_Node * arg;
			struct AST_Node * next;
		} call_args;
	};
} AST_Node;

void ast_pretty_print(AST_Node const * program);

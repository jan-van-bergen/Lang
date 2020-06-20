#pragma once
#include "Token.h"

typedef enum AST_Statement_Type {
	AST_STATEMENT_NOOP,
	AST_STATEMENTS,

	AST_STATEMENT_EXPR,
	AST_STATEMENT_DECL_VAR,
	AST_STATEMENT_DECL_FUNC,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,
	AST_STATEMENT_RETURN
} AST_Statement_Type;

typedef enum AST_Expression_Type {
	AST_EXPRESSION_CONST,
	AST_EXPRESSION_VAR,
	AST_EXPRESSION_ASSIGN,
	AST_EXPRESSION_OPERATOR_BIN,
	AST_EXPRESSION_OPERATOR_PRE,
	AST_EXPRESSION_OPERATOR_POST,
	AST_EXPRESSION_CALL_FUNC
} AST_Expression_Type;

typedef struct AST_Decl_Arg {
	char const * name;
	char const * type;
	struct AST_Decl_Arg * next;
} AST_Decl_Arg;

typedef struct AST_Call_Arg {
	struct AST_Expression * expr;
	struct AST_Call_Arg   * next;
} AST_Call_Arg;

typedef struct AST_Statement {
	AST_Statement_Type type;

	union {
		struct Statements {
			struct AST_Statement * head;
			struct AST_Statement * cons;
		} stat_statements;

		struct Decl {
			char const * name;
			char const * type;
		} stat_decl;

		struct Assign {
			char const * name;
			struct AST_Expression * expr;
		} stat_assign;

		struct Expr {
			struct AST_Expression * expr;
		} stat_expr;

		struct Func {
			char const * name;
			char const * return_type;
			struct AST_Decl_Args * args;
			struct AST_Statement * body;
		} stat_func;

		struct If {
			struct AST_Expression * condition;

			struct AST_Statement * case_true;
			struct AST_Statement * case_false;
		} stat_if;

		struct While {
			struct AST_Expression * condition;
			struct AST_Statement  * body;
		} stat_while;

		struct Return {
			struct AST_Expression * expr;
		} stat_return;
	};
} AST_Statement;

typedef struct AST_Expression {
	AST_Expression_Type type;

	union {
		struct Const {
			Token token;
		} expr_const;

		struct Var {
			Token token;
		} expr_var;

		struct Op_Bin {
			Token token;

			struct AST_Expression * expr_left;
			struct AST_Expression * expr_right;
		} expr_op_bin;

		struct Op_Pre {
			Token token;

			struct AST_Expression * expr;
		} expr_op_pre;
		
		struct Op_Post {
			Token token;

			struct AST_Expression * expr;
		} expr_op_post;

		struct Call {
			char const * function;

			struct AST_Call_Arg * args;
		} expr_call;
	};
} AST_Expression;

void ast_pretty_print(AST_Statement const * program);

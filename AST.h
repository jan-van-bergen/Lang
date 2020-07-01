#pragma once
#include "Token.h"

#include "Type.h"
#include "Scope.h"

typedef enum AST_Expression_Type {
	AST_EXPRESSION_CONST,
	AST_EXPRESSION_VAR,
	AST_EXPRESSION_STRUCT_MEMBER,
	AST_EXPRESSION_CAST,
	AST_EXPRESSION_SIZEOF,
	AST_EXPRESSION_ASSIGN,
	AST_EXPRESSION_OPERATOR_BIN,
	AST_EXPRESSION_OPERATOR_PRE,
	AST_EXPRESSION_OPERATOR_POST,
	AST_EXPRESSION_CALL_FUNC
} AST_Expression_Type;

typedef struct AST_Call_Arg {
	struct AST_Expression * expr;
	struct AST_Call_Arg   * next;

	int height;
} AST_Call_Arg;

typedef struct AST_Expression {
	AST_Expression_Type type;

	int height;

	union {
		struct Const {
			Token token;
		} expr_const;

		struct Var {
			char const * name;
		} expr_var;

		struct Struct_Member {
			struct AST_Expression * expr;
			char const            * member_name;
		} expr_struct_member;

		struct Cast {
			Type * new_type;
			struct AST_Expression * expr;
		} expr_cast;

		struct Sizeof {
			Type * type;
		} expr_sizeof;

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
			char const * function_name;

			struct AST_Call_Arg * args;
		} expr_call;
	};
} AST_Expression;

typedef enum AST_Statement_Type {
	AST_STATEMENT_PROGRAM,

	AST_STATEMENTS,
	AST_STATEMENT_BLOCK,

	AST_STATEMENT_EXPR,
	AST_STATEMENT_DEF_VAR,
	AST_STATEMENT_DEF_FUNC,
	AST_STATEMENT_EXTERN,
	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,
	AST_STATEMENT_BREAK,
	AST_STATEMENT_CONTINUE,
	AST_STATEMENT_RETURN
} AST_Statement_Type;

typedef struct AST_Def_Arg {
	char const * name;
	Type       * type;
	struct AST_Def_Arg * next;
} AST_Def_Arg;

typedef struct AST_Def_Func {
	char const * name;
	Type       * return_type;

	Variable_Buffer * buffer_args;
	Variable_Buffer * buffer_vars;

	Scope * scope_args;

	int arg_count;

	struct AST_Def_Arg  * args;
	struct AST_Statement * body;
} AST_Def_Func;

typedef struct AST_Statement {
	AST_Statement_Type type;

	union {
		struct Program {
			Variable_Buffer * globals;
			Scope           * global_scope;

			struct AST_Statement * stat;
		} stat_program;

		struct Statements {
			struct AST_Statement * head;
			struct AST_Statement * cons;
		} stat_stats;
		
		struct Block {
			Scope * scope;

			struct AST_Statement * stat;
		} stat_block;

		struct Expr {
			struct AST_Expression * expr;
		} stat_expr;

		struct Def_Var {
			char const * name;
			Type       * type;

			struct AST_Expression * assign;
		} stat_def_var;

		AST_Def_Func stat_def_func;

		AST_Def_Func stat_extern;

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

void ast_pretty_print(AST_Statement const * program);

void ast_free_statement(AST_Statement * stat);

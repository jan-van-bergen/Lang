#pragma once
#include "Token.h"

typedef enum AST_Type {
	AST_STATEMENTS,
	AST_STATEMENT_DECL,
	AST_STATEMENT_ASSIGN,
	AST_STATEMENT_IF,

	AST_EXPRESSION_CONST,
	AST_EXPRESSION_VAR,
	AST_EXPRESSION_OPERATOR_BIN,
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

		struct If {
			struct AST_Node * condition;

			struct AST_Node * case_true;
			struct AST_Node * case_false;
		} stat_if;

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
	};
} AST_Node;

// void ast_debug(AST_Statement const * program);

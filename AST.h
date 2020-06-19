#pragma once
#include "Token.h"

typedef enum AST_Expression_Type {
	AST_EXPRESSION_CONST,
	AST_EXPRESSION_VAR,
	//AST_EXPRESSION_OPERATOR_BIN,
	//AST_EXPRESSION_OPERATOR_PRE,
	//AST_EXPRESSION_OPERATOR_POST,
	//AST_EXPRESSION_FUNCTION_CALL
} AST_Expression_Type;

typedef struct AST_Expression {
	AST_Expression_Type type;

	union {
		struct Const {
			Token token;
		} expr_const;

		struct Var {
			Token token;
		} expr_var;
	};
} AST_Expression;

typedef enum AST_Statement_Type {
	AST_STATEMENT_DECL,
	AST_STATEMENT_ASSIGN,
	AST_STATEMENT_IF,
	AST_STATEMENT_BLOCK
} AST_Statement_Type;

typedef struct AST_Statement {
	AST_Statement_Type type;

	union {
		struct Decl {
			char const * name;
			char const * type;
			struct AST_Expression * expr;
		} stat_decl;

		struct Assign {
			char const * name;
			struct AST_Expression * expr;
		} stat_assign;

		struct If {
			struct AST_Expression * condition;

			struct AST_Statement * case_true;
			struct AST_Statement * case_false;
		} stat_if;

		struct Block {
			int                    statement_count;
			struct AST_Statement * statements;
		} stat_block;
	};
} AST_Statement;

void ast_debug(AST_Statement const * program);

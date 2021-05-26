#pragma once
#include "Token.h"

#include "Type.h"
#include "Scope.h"

typedef enum Operator_Bin {
	OPERATOR_BIN_ASSIGN      = TOKEN_ASSIGN,
	OPERATOR_BIN_LOGICAL_AND = TOKEN_OPERATOR_LOGICAL_AND,
	OPERATOR_BIN_LOGICAL_OR	 = TOKEN_OPERATOR_LOGICAL_OR,
	OPERATOR_BIN_PLUS		 = TOKEN_OPERATOR_PLUS,
	OPERATOR_BIN_MINUS		 = TOKEN_OPERATOR_MINUS,
	OPERATOR_BIN_MULTIPLY	 = TOKEN_OPERATOR_MULTIPLY,
	OPERATOR_BIN_DIVIDE		 = TOKEN_OPERATOR_DIVIDE,
	OPERATOR_BIN_MODULO		 = TOKEN_OPERATOR_MODULO,
	OPERATOR_BIN_SHIFT_LEFT	 = TOKEN_OPERATOR_SHIFT_LEFT,
	OPERATOR_BIN_SHIFT_RIGHT = TOKEN_OPERATOR_SHIFT_RIGHT,
	OPERATOR_BIN_LT			 = TOKEN_OPERATOR_LT,
	OPERATOR_BIN_LE			 = TOKEN_OPERATOR_LT_EQ,
	OPERATOR_BIN_GT			 = TOKEN_OPERATOR_GT,
	OPERATOR_BIN_GE			 = TOKEN_OPERATOR_GT_EQ,
	OPERATOR_BIN_EQ			 = TOKEN_OPERATOR_EQ,
	OPERATOR_BIN_NE			 = TOKEN_OPERATOR_NE,
	OPERATOR_BIN_BITWISE_AND = TOKEN_OPERATOR_BITWISE_AND,
	OPERATOR_BIN_BITWISE_XOR = TOKEN_OPERATOR_BITWISE_XOR,
	OPERATOR_BIN_BITWISE_OR	 = TOKEN_OPERATOR_BITWISE_OR
} Operator_Bin;

typedef enum Operator_Pre {
	OPERATOR_PRE_ADDRESS_OF  = TOKEN_OPERATOR_BITWISE_AND,
	OPERATOR_PRE_DEREF       = TOKEN_OPERATOR_MULTIPLY,
	OPERATOR_PRE_INC         = TOKEN_OPERATOR_INC,
	OPERATOR_PRE_DEC         = TOKEN_OPERATOR_DEC,
	OPERATOR_PRE_PLUS        = TOKEN_OPERATOR_PLUS,
	OPERATOR_PRE_MINUS       = TOKEN_OPERATOR_MINUS,
	OPERATOR_PRE_LOGICAL_NOT = TOKEN_OPERATOR_LOGICAL_NOT,
	OPERATOR_PRE_BITWISE_NOT = TOKEN_OPERATOR_BITWISE_NOT
} Operator_Pre;

typedef enum Operator_Post {
	OPERATOR_POST_INC = TOKEN_OPERATOR_INC,
	OPERATOR_POST_DEC = TOKEN_OPERATOR_DEC
} Operator_Post;

char const * operator_bin_to_str (Operator_Bin  operator);
char const * operator_pre_to_str (Operator_Pre  operator);
char const * operator_post_to_str(Operator_Post operator);

typedef enum AST_Expression_Type {
	AST_EXPRESSION_CONST,
	AST_EXPRESSION_VAR,
	AST_EXPRESSION_ARRAY_ACCESS,
	AST_EXPRESSION_STRUCT_MEMBER,
	AST_EXPRESSION_CAST,
	AST_EXPRESSION_SIZEOF,
	AST_EXPRESSION_OPERATOR_BIN,
	AST_EXPRESSION_OPERATOR_PRE,
	AST_EXPRESSION_OPERATOR_POST,
	AST_EXPRESSION_CALL_FUNC
} AST_Expression_Type;

typedef struct AST_Call_Arg {
	int height;

	struct AST_Expression * expr;
} AST_Call_Arg;

typedef struct AST_Expression {
	AST_Expression_Type type;

	int line;
	int height;

	union {
		struct Const {
			Token token;
		} expr_const;

		struct Var {
			char const * name;
		} expr_var;

		struct Array_Access {
			struct AST_Expression * expr_array;
			struct AST_Expression * expr_index;
		} expr_array_access;

		struct Struct_Member {
			struct AST_Expression * expr;
			char const            * member_name;
		} expr_struct_member;

		struct Cast {
			Type const * new_type;
			struct AST_Expression * expr;
		} expr_cast;

		struct Sizeof {
			Type const * type;
		} expr_sizeof;

		struct Op_Bin {
			Operator_Bin operator;

			struct AST_Expression * expr_left;
			struct AST_Expression * expr_right;
		} expr_op_bin;

		struct Op_Pre {
			Operator_Pre operator;

			struct AST_Expression * expr;
		} expr_op_pre;
		
		struct Op_Post {
			Operator_Post operator;

			struct AST_Expression * expr;
		} expr_op_post;

		struct Call {
			struct AST_Expression * expr_function;

			int                   arg_count;
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
	AST_STATEMENT_EXPORT,

	AST_STATEMENT_IF,
	AST_STATEMENT_WHILE,

	AST_STATEMENT_BREAK,
	AST_STATEMENT_CONTINUE,
	AST_STATEMENT_RETURN
} AST_Statement_Type;

typedef struct AST_Def_Arg {
	char const * name;
	Type const * type;
} AST_Def_Arg;

typedef struct AST_Statement {
	AST_Statement_Type type;

	int line;

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
			Type const * type;

			struct AST_Expression * assign;
		} stat_def_var;

		struct Def_Func {
			Function_Def * function_def;

			Variable_Buffer * buffer_args;
			Variable_Buffer * buffer_vars;

			Scope * scope_args;

			struct AST_Statement * body;
		} stat_def_func;

		struct Extern {
			Function_Def * function_def;
		} stat_extern;
		
		struct Export {
			char const * name;
		} stat_export;

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

AST_Expression * ast_make_expr_const(int line, Token const * token);
AST_Expression * ast_make_expr_var  (int line, char  const * name);

AST_Expression * ast_make_expr_array_access (int line, AST_Expression * expr_array, AST_Expression * expr_index);
AST_Expression * ast_make_expr_struct_member(int line, AST_Expression * expr_struct, char  const * member_name);

AST_Expression * ast_make_expr_cast  (int line, Type const * type, AST_Expression * expr);
AST_Expression * ast_make_expr_sizeof(int line, Type const * type);

AST_Expression * ast_make_expr_op_bin (int line, Operator_Bin  operator, AST_Expression * expr_left, AST_Expression * expr_right);
AST_Expression * ast_make_expr_op_pre (int line, Operator_Pre  operator, AST_Expression * expr);
AST_Expression * ast_make_expr_op_post(int line, Operator_Post operator, AST_Expression * expr);

AST_Expression * ast_make_expr_call(int line, AST_Expression * expr_function, int arg_count, AST_Call_Arg * args); 


AST_Statement * ast_make_stat_program(int line, Variable_Buffer * globals, Scope * global_scope, AST_Statement * stat);

AST_Statement * ast_make_stat_stats(int line, AST_Statement * head, AST_Statement * cons);
AST_Statement * ast_make_stat_block(int line, Scope * scope, AST_Statement * stat);

AST_Statement * ast_make_stat_expr(int line, AST_Expression * expr);

AST_Statement * ast_make_stat_def_var (int line, char const * name, Type const * type, AST_Expression * assign);
AST_Statement * ast_make_stat_def_func(int line, Function_Def * function_def, Variable_Buffer * buffer_args, Variable_Buffer * buffer_vars, Scope * scope_args, AST_Statement * body);
AST_Statement * ast_make_stat_extern  (int line, Function_Def * function_def);
AST_Statement * ast_make_stat_export  (int line, char const * name);

AST_Statement * ast_make_stat_if   (int line, AST_Expression * condition, AST_Statement * case_true, AST_Statement * case_false);
AST_Statement * ast_make_stat_while(int line, AST_Expression * condition, AST_Statement * body);

AST_Statement * ast_make_stat_break   (int line);
AST_Statement * ast_make_stat_continue(int line);
AST_Statement * ast_make_stat_return  (int line, AST_Expression * expr);


bool ast_is_lvalue(AST_Expression const * expr);

bool ast_contains(AST_Expression const * expr, AST_Expression_Type expression_type);


void ast_print_expression(AST_Expression const * expr, char * string, int string_size);
void ast_print_statement (AST_Statement  const * stat, char * string, int string_size);


void ast_free_statement(AST_Statement * stat);

typedef enum Precedence {
    PRECEDENCE_NONE,           // Lowest precedence
    PRECEDENCE_ARRAY_ACCESS,
    PRECEDENCE_UNARY_POST,
    PRECEDENCE_UNARY_PRE,
    PRECEDENCE_CAST_SIZEOF,
    PRECEDENCE_MULTIPLICATIVE,
    PRECEDENCE_ADDITIVE,
    PRECEDENCE_SHIFT,
    PRECEDENCE_RELATIONAL,
    PRECEDENCE_EQUALITY,
    PRECEDENCE_BITWISE_AND,
    PRECEDENCE_BITWISE_XOR,
    PRECEDENCE_BITWISE_OR,
    PRECEDENCE_LOGICAL_AND,
    PRECEDENCE_LOGICAL_OR,
    PRECEDENCE_ASSIGNMENT      // Highest precedence
} Precedence;

int get_precedence(AST_Expression const * expr);

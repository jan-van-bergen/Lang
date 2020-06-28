#pragma once
#include "Type.h"

#include <stdbool.h>

typedef struct Variable {
	char const * name;
	Type       * type;

	bool is_global;

	int offset;
} Variable;

int variable_get_size(Variable const * var);

typedef struct Stack_Frame {
	char const * function_name;

	int        vars_len;
	int        vars_cap;
	Variable * vars;

	int arg_size; // In bytes
	int var_size;

	int curr_arg_offset; // Used during codegen
	int curr_var_offset;
} Stack_Frame;

Stack_Frame * make_stack_frame(char const * function_name);
void          free_stack_frame(Stack_Frame * stack_frame);

typedef struct Scope {
	struct Scope * prev;

	int   indices_len;
	int   indices_cap;
	int * indices;

	Stack_Frame * stack_frame;
} Scope;

Scope * make_scope(Stack_Frame * stack_frame);
void    free_scope(Scope * scope);

bool scope_is_global(Scope const * scope);

void scope_add_var(Scope * scope, char const * name, Type * type);
void scope_add_arg(Scope * scope, char const * name, Type * type);

Variable * scope_get_variable(Scope const * scope, char const * name);

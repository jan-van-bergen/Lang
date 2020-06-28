#include "Scope.h"

#include <assert.h>
#include <string.h>

#include <stdlib.h>
#include <stdbool.h>

int variable_get_size(Variable const * var) {
	return 8; // TEMP


	switch (var->type->type) {
		case TYPE_INT:     return 8; // 64 bit
		case TYPE_BOOL:    return 1;
		case TYPE_CHAR:    return 1;
		case TYPE_POINTER: return 8;

		default: abort();
	}
}

static int stack_frame_add_variable(Stack_Frame * frame, char const * name, Type * type, bool is_global) {
	if (frame->vars_len == frame->vars_cap) {
		frame->vars_cap *= 2;
		frame->vars = realloc(frame->vars, frame->vars_cap * sizeof(Variable));
	}

	int index = frame->vars_len++;
	
	Variable * var = &frame->vars[index];
	var->name = name;
	var->type = type;
	var->is_global = is_global;

	return index;
}

Stack_Frame * make_stack_frame(char const * function_name) {
	Stack_Frame * frame = malloc(sizeof(Stack_Frame));
	frame->function_name = function_name;

	frame->vars_len = 0;
	frame->vars_cap = 16;
	frame->vars = malloc(frame->vars_cap * sizeof(Variable));

	frame->arg_size = 0;
	frame->var_size = 0;

	frame->curr_arg_offset = 8; // Bias by 8 to account for return address on stack
	frame->curr_var_offset = 8; // Bias by 8 to account for stack frame on stack

	return frame;
}

void free_stack_frame(Stack_Frame * stack_frame) {
	free(stack_frame->vars);
	free(stack_frame);
}

Scope * make_scope(Stack_Frame * stack_frame) {
	Scope * scope = malloc(sizeof(Scope));
	scope->prev = NULL;

	scope->indices_len = 0;
	scope->indices_cap = 16;
	scope->indices = malloc(scope->indices_cap * sizeof(int));

	scope->stack_frame = stack_frame;
	
	return scope;
}

void free_scope(Scope * scope) {
	free(scope->indices);
	free(scope);
}

bool scope_is_global(Scope const * scope) {
	return scope->prev == NULL;
}

void scope_add_arg(Scope * scope, char const * name, Type * type) {
	assert(!scope_is_global(scope));

	int index = stack_frame_add_variable(scope->stack_frame, name, type, false);
	
	if (scope->indices_len == scope->indices_cap) {
		scope->indices_cap *= 2;
		scope->indices = realloc(scope->indices, scope->indices_cap * sizeof(int));
	}

	scope->indices[scope->indices_len++] = index;

	Variable * arg = &scope->stack_frame->vars[index];
	scope->stack_frame->arg_size += variable_get_size(arg);
}

void scope_add_var(Scope * scope, char const * name, Type * type) {
	int index = stack_frame_add_variable(scope->stack_frame, name, type, scope_is_global(scope));

	if (scope->indices_len == scope->indices_cap) {
		scope->indices_cap *= 2;
		scope->indices = realloc(scope->indices, scope->indices_cap * sizeof(int));
	}

	scope->indices[scope->indices_len++] = index;

	Variable * var = &scope->stack_frame->vars[index];
	scope->stack_frame->var_size += variable_get_size(var);
}

Variable * scope_get_variable(Scope const * scope, char const * name) {
	while (true) {
		Stack_Frame const * frame = scope->stack_frame;

		for (int i = 0; i < scope->indices_len; i++) {
			int index = scope->indices[i];

			if (strcmp(frame->vars[index].name, name) == 0) {
				return frame->vars + index;
			}
		}

		scope = scope->prev;

		if (scope == NULL) abort(); // Variable not found!
	}
}

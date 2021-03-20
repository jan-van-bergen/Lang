#include "Scope.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Error.h"

static int variable_buffer_add_variable(Variable_Buffer * buf, char const * name, Type const * type, bool is_global) {
	if (buf->vars_len == buf->vars_cap) {
		buf->vars_cap *= 2;
		buf->vars = realloc(buf->vars, buf->vars_cap * sizeof(Variable));
	}

	int index = buf->vars_len++;
	
	Variable * var = &buf->vars[index];
	var->name = name;
	var->type = type;
	var->is_global = is_global;

	return index;
}

Variable_Buffer * make_variable_buffer(char const * name) {
	Variable_Buffer * buf = malloc(sizeof(Variable_Buffer));
	buf->name = name;

	buf->vars_len = 0;
	buf->vars_cap = 16;
	buf->vars = malloc(buf->vars_cap * sizeof(Variable));

	buf->size  = 0;
	buf->align = 0;

	return buf;
}

void free_variable_buffer(Variable_Buffer * list) {
	free(list->vars);
	free(list);
}

Scope * make_scope(Variable_Buffer * buf) {
	Scope * scope = malloc(sizeof(Scope));
	scope->prev = NULL;

	scope->indices_len = 0;
	scope->indices_cap = 16;
	scope->indices = malloc(scope->indices_cap * sizeof(int));

	scope->variable_buffer = buf;
	
	scope->struct_defs_len = 0;
	scope->struct_defs_cap = 16;
	scope->struct_defs = malloc(scope->struct_defs_cap * sizeof(Struct_Def));

	scope->function_defs_len = 0;
	scope->function_defs_cap = 16;
	scope->function_defs = malloc(scope->function_defs_cap * sizeof(Function_Def));

	return scope;
}

void free_scope(Scope * scope) {
	free(scope->indices);
	free(scope);
}

bool scope_is_global(Scope const * scope) {
	return scope->prev == NULL;
}

static Variable * scope_lookup(Scope const * scope, char const * name) {
	for (int i = 0; i < scope->indices_len; i++) {
		int index = scope->indices[i];

		if (strcmp(scope->variable_buffer->vars[index].name, name) == 0) {
			return scope->variable_buffer->vars + index;
		}
	}

	return NULL; // Not found
}

void scope_add_arg(Scope * scope, char const * name, Type const * type) {
	assert(!scope_is_global(scope));
	
	// Check if a variable with this name already exists
	if (scope_lookup(scope, name)) {
		printf("ERROR: Argument with name '%s' already exists at this scope!\n", name);
		exit(ERROR_SCOPE);
	}

	int index = variable_buffer_add_variable(scope->variable_buffer, name, type, false);
	
	if (scope->indices_len == scope->indices_cap) {
		scope->indices_cap *= 2;
		scope->indices = realloc(scope->indices, scope->indices_cap * sizeof(int));
	}

	scope->indices[scope->indices_len++] = index;

	Variable * arg = &scope->variable_buffer->vars[index];
	
	int arg_size  = type_get_size (arg->type, scope);
	int arg_align = type_get_align(arg->type, scope);

	if (scope->indices_len <= 4) { // First 4 arguments will be put into shadow space by callee
		arg_size  = 8;
		arg_align = 8;
	}

	align(&scope->variable_buffer->size, arg_align);
		
	arg->offset = scope->variable_buffer->size;
	scope->variable_buffer->size += arg_size;

	if (scope->variable_buffer->align < arg_align) {
		scope->variable_buffer->align = arg_align;
	}
}

void scope_add_var(Scope * scope, char const * name, Type const * type) {
	// Check if a variable with this name already exists
	if (scope_lookup(scope, name)) {
		printf("ERROR: Variable with name '%s' already exists at this scope!\n", name);
		exit(ERROR_SCOPE);
	}

	int index = variable_buffer_add_variable(scope->variable_buffer, name, type, scope_is_global(scope));

	if (scope->indices_len == scope->indices_cap) {
		scope->indices_cap *= 2;
		scope->indices = realloc(scope->indices, scope->indices_cap * sizeof(int));
	}

	scope->indices[scope->indices_len++] = index;

	Variable * var = &scope->variable_buffer->vars[index];

	int var_size  = type_get_size (var->type, scope);
	int var_align = type_get_align(var->type, scope);

	align(&scope->variable_buffer->size, var_align);

	var->offset = scope->variable_buffer->size;
	scope->variable_buffer->size += var_size;

	if (scope->variable_buffer->align < var_align) {
		scope->variable_buffer->align = var_align;
	}
}

Struct_Def * scope_add_struct_def(Scope * scope) {
	if (scope->struct_defs_len == scope->struct_defs_cap) {
		scope->struct_defs_cap *= 2;
		scope->struct_defs = realloc(scope->struct_defs, scope->struct_defs_cap * sizeof(Struct_Def));
	}

	Struct_Def ** struct_def = scope->struct_defs + scope->struct_defs_len++;
	*struct_def = malloc(sizeof(Struct_Def));

	return *struct_def; 
}

Function_Def * scope_add_function_def(Scope * scope) {
	if (scope->function_defs_len == scope->function_defs_cap) {
		scope->function_defs_cap *= 2;
		scope->function_defs = realloc(scope->function_defs, scope->function_defs_cap * sizeof(Struct_Def));
	}
	
	Function_Def ** function_def = scope->function_defs + scope->function_defs_len++;
	*function_def = malloc(sizeof(Function_Def));

	return * function_def;
}

Variable * scope_get_variable(Scope const * scope, char const * name) {
	while (true) {
		Variable * var = scope_lookup(scope, name);
		if (var) return var;

		scope = scope->prev;

		if (scope == NULL) {
			printf("Error: Variable '%s' not defined or is not in scope!", name);
			exit(ERROR_SCOPE);
		}
	}
}

Struct_Def * scope_get_struct_def(Scope const * scope, char const * name) {
	while (true) {
		for (int i = 0; i < scope->struct_defs_len; i++) {
			if (strcmp(scope->struct_defs[i]->name, name) == 0) {
				return scope->struct_defs[i];
			}
		}

		scope = scope->prev;

		if (scope == NULL) {
			printf("ERROR: Struct '%s' not defined or is not in scope!", name);
			exit(ERROR_SCOPE);
		}
	}
}

Function_Def * scope_get_function_def(Scope const * scope, char const * name) {
	while (true) {
		for (int i = 0; i < scope->function_defs_len; i++) {
			if (strcmp(scope->function_defs[i]->name, name) == 0) {
				return scope->function_defs[i];
			}
		}
	
		scope = scope->prev;

		if (scope == NULL) {
			printf("ERROR: Function '%s' is not defined or not in scope!", name);
			exit(ERROR_SCOPE);
		}
	}
}
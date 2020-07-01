#pragma once
#include "Type.h"

#include <stdbool.h>

typedef struct Variable {
	char const * name;
	Type       * type;

	bool is_global;

	int offset;
} Variable;

typedef struct Variable_Buffer {
	char const * name;

	int        vars_len;
	int        vars_cap;
	Variable * vars;

	int size;  // Total span in bytes (including padding due to alignment)
	int align; // Alignment of largest element
} Variable_Buffer;

Variable_Buffer * make_variable_buffer(char const * name);
void              free_variable_buffer(Variable_Buffer * variable_buffer);


typedef struct Struct_Def {
	char const * name;

	struct Variable_Buffer * members;
	struct Scope           * member_scope;
} Struct_Def;


typedef struct Function_Def {
	char const * name;
	Type       * return_type;

	int                   arg_count;
	struct AST_Def_Arg  * args;
} Function_Def;

typedef struct Scope {
	struct Scope * prev;

	int   indices_len;
	int   indices_cap;
	int * indices; // Index into the Stack_Frame's Variable array

	Variable_Buffer * variable_buffer;

	int          struct_defs_len;
	int          struct_defs_cap;
	Struct_Def * struct_defs;

	int            function_defs_len;
	int            function_defs_cap;
	Function_Def * function_defs;
} Scope;

Scope * make_scope(Variable_Buffer * variable_buffer);
void    free_scope(Scope * scope);

bool scope_is_global(Scope const * scope);

void scope_add_arg(Scope * scope, char const * name, Type * type);
void scope_add_var(Scope * scope, char const * name, Type * type);

Struct_Def   * scope_add_struct_def  (Scope * scope);
Function_Def * scope_add_function_def(Scope * scope);

Variable     * scope_get_variable    (Scope const * scope, char const * name);
Struct_Def   * scope_get_struct_def  (Scope const * scope, char const * name);
Function_Def * scope_get_function_def(Scope const * scope, char const * name);

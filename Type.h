#pragma once
#include <stdbool.h>

typedef enum Type_Type {
	TYPE_VOID,

	TYPE_INT,
	TYPE_BOOL,
	TYPE_CHAR,

	TYPE_POINTER
} Type_Type;

typedef struct Type {
	Type_Type type;

	struct Type * ptr; // If type is a TYPE_POINTER, this contains the Type being pointed to
} Type;


Type * make_type_void();

Type * make_type_int();
Type * make_type_bool();
Type * make_type_char();

Type * make_type_pointer(Type const * type);


void type_to_string(Type const * type, char * string, int string_size);


int type_get_size(Type const * type);


bool type_is_void    (Type const * type);
bool type_is_integral(Type const * type);
bool type_is_boolean (Type const * type);
bool type_is_pointer (Type const * type);


bool   types_unifiable(Type const * a, Type const * b);
Type * types_unify(Type const * a, Type const * b);


//typedef struct Function_Type {
//
//
//	int     arg_count;
//	Type ** arg_types;
//
//	Type * return_type;
//} Function_Type;

#pragma once
#include <stdbool.h>

typedef enum Type_Type {
	TYPE_VOID,

	TYPE_I8,
	TYPE_I16,
	TYPE_I32,
	TYPE_I64,

	TYPE_U8,
	TYPE_U16,
	TYPE_U32,
	TYPE_U64,

	TYPE_BOOL,

	TYPE_STRUCT
} Type_Type;

typedef struct Type {
	Type_Type type;

	int ptr_level; // Number of pointer indirections, 0 means not a pointer

	union {
		char const * struct_name; // If type is a TYPE_STRUCT
	};
} Type;


Type make_type_void();

Type make_type_i8 ();
Type make_type_i16();
Type make_type_i32();
Type make_type_i64();

Type make_type_u8 ();
Type make_type_u16();
Type make_type_u32();
Type make_type_u64();

Type make_type_bool();

Type make_type_pointer(Type const * type);
Type make_type_deref  (Type const * type);

Type make_type_str(); // char *

void type_to_string(Type const * type, char * string, int string_size);


int type_get_size (Type const * type, struct Scope * scope);
int type_get_align(Type const * type, struct Scope * scope);


void align(int * address, int alignment);


bool type_is_void(Type const * type);

bool type_is_signed_integral  (Type const * type);
bool type_is_unsigned_integral(Type const * type);
bool type_is_integral(Type const * type);

bool type_is_boolean(Type const * type);

bool type_is_pointer(Type const * type);

bool type_is_struct(Type const * type);

bool type_is_void_pointer(Type const * type);

bool type_is_primitive(Type const * type);


bool types_equal(Type const * a, Type const * b);


bool types_unifiable(Type const * a, Type const * b);
Type types_unify    (Type const * a, Type const * b, struct Scope * scope);

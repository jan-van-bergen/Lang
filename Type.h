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

	TYPE_POINTER,

	TYPE_STRUCT
} Type_Type;

typedef struct Type {
	Type_Type type;

	union {
		struct Type const * ptr;         // If type is a TYPE_POINTER
		char        const * struct_name; // If type is a TYPE_STRUCT
	};
} Type;

void type_table_init();
void type_table_free();

Type * type_table_new_type();

Type const * make_type_void();

Type const * make_type_i8 ();
Type const * make_type_i16();
Type const * make_type_i32();
Type const * make_type_i64();

Type const * make_type_u8 ();
Type const * make_type_u16();
Type const * make_type_u32();
Type const * make_type_u64();

Type const * make_type_bool();

Type const * make_type_pointer(Type const * type);


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


bool         types_unifiable(Type const * a, Type const * b);
Type const * types_unify    (Type const * a, Type const * b, struct Scope * scope);

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

	TYPE_F32,
	TYPE_F64,

	TYPE_BOOL,

	TYPE_POINTER,

	TYPE_FUNCTION,

	TYPE_ARRAY,
	TYPE_STRUCT
} Type_Type;

typedef struct Type {
	Type_Type type;
	
	struct Type const * base;  // NULL if type is primitive

	union {
		int array_size;           // Active if type == TYPE_ARRAY

		char const * struct_name; // Active if type == TYPE_STRUCT

		struct {
			struct Type const ** args;
			int                  arg_count;

			struct Type const * return_type;
		} function;
	};
} Type;

void type_table_init();
void type_table_mem_free();

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

Type const * make_type_f32();
Type const * make_type_f64();

Type const * make_type_bool();

Type const * make_type_array  (Type const * base_type, int size);
Type const * make_type_pointer(Type const * base_type);

Type const * make_type_function(Type const ** arg_types, int arg_count, Type const * return_type);

int type_to_string(Type const * type, char * string, int string_size);


int type_get_size (Type const * type, struct Scope * scope);
int type_get_align(Type const * type, struct Scope * scope);


void align(int * address, int alignment);


bool type_is_void(Type const * type);

bool type_is_i8 (Type const * type);
bool type_is_i16(Type const * type);
bool type_is_i32(Type const * type);
bool type_is_i64(Type const * type);

bool type_is_u8 (Type const * type);
bool type_is_u16(Type const * type);
bool type_is_u32(Type const * type);
bool type_is_u64(Type const * type);

bool type_is_f32(Type const * type);
bool type_is_f64(Type const * type);

bool type_is_bool(Type const * type);

bool type_is_array  (Type const * type);
bool type_is_pointer(Type const * type);

bool type_is_function(Type const * type);

bool type_is_struct(Type const * type);

bool type_is_integral_signed  (Type const * type);
bool type_is_integral_unsigned(Type const * type);
bool type_is_integral         (Type const * type);

bool type_is_float(Type const * type);

bool type_is_arithmetic(Type const * type);

bool type_is_void_pointer(Type const * type);
bool type_is_string(Type const * type);

bool type_is_primitive(Type const * type);

bool type_is_aggregate(Type const * type);

bool types_equal(Type const * a, Type const * b);


bool         types_unifiable(Type const * a, Type const * b);
Type const * types_unify    (Type const * a, Type const * b, struct Scope * scope);

Type const * type_dereference(Type const * ptr_type);

Type const * type_infer(struct AST_Expression const * expr, struct Scope const * scope);

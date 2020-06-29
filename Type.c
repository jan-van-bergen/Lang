#include "Type.h"

#include <string.h>

#include <stdio.h>
#include <stdlib.h>

static Type type_void = { TYPE_VOID, NULL };

static Type type_i8   = { TYPE_I8,  NULL };
static Type type_i16  = { TYPE_I16, NULL };
static Type type_i32  = { TYPE_I32, NULL };
static Type type_i64  = { TYPE_I64, NULL };

static Type type_u8   = { TYPE_U8,  NULL };
static Type type_u16  = { TYPE_U16, NULL };
static Type type_u32  = { TYPE_U32, NULL };
static Type type_u64  = { TYPE_U64, NULL };

static Type type_bool = { TYPE_BOOL, NULL };

Type * make_type_void() { return &type_void; }

Type * make_type_i8 () { return &type_i8; }
Type * make_type_i16() { return &type_i16; }
Type * make_type_i32() { return &type_i32; }
Type * make_type_i64() { return &type_i64; }

Type * make_type_u8 () { return &type_u8; }
Type * make_type_u16() { return &type_u16; }
Type * make_type_u32() { return &type_u32; }
Type * make_type_u64() { return &type_u64; }

Type * make_type_bool() { return &type_bool; }

Type * make_type_pointer(Type const * type) {
	Type * ptr_type = malloc(sizeof(Type));
	ptr_type->type = TYPE_POINTER;
	ptr_type->ptr  = type;

	return ptr_type;
}

void type_to_string(Type const * type, char * string, int string_size) {
	switch (type->type) {
		case TYPE_VOID: sprintf_s(string, string_size, "void"); break;

		case TYPE_I8:  sprintf_s(string, string_size, "i8");  break;
		case TYPE_I16: sprintf_s(string, string_size, "i16"); break;
		case TYPE_I32: sprintf_s(string, string_size, "i32"); break;
		case TYPE_I64: sprintf_s(string, string_size, "i64"); break;

		case TYPE_U8:  sprintf_s(string, string_size, "u8");  break;
		case TYPE_U16: sprintf_s(string, string_size, "u16"); break;
		case TYPE_U32: sprintf_s(string, string_size, "u32"); break;
		case TYPE_U64: sprintf_s(string, string_size, "u64"); break;

		case TYPE_BOOL: sprintf_s(string, string_size, "bool"); break;

		case TYPE_POINTER: {
			type_to_string(type->ptr, string, string_size);
			strcat_s(string, string_size, " *");

			break;
		}

		default: abort();
	}
}

int type_get_size(Type const * type) {
	switch (type->type) {
		case TYPE_VOID: abort(); // Invalid!

		case TYPE_I8:  return 1;
		case TYPE_I16: return 2;
		case TYPE_I32: return 4;
		case TYPE_I64: return 8;

		case TYPE_U8:  return 1;
		case TYPE_U16: return 2;
		case TYPE_U32: return 4;
		case TYPE_U64: return 8;

		case TYPE_BOOL: return 1;

		case TYPE_POINTER: return 8;

		default: abort();
	}
}


bool type_is_void(Type const * type) {
	return type->type == TYPE_VOID;
}

bool type_is_signed_integral(Type const * type) {
	return type->type == TYPE_I8 || type->type == TYPE_I16 || type->type == TYPE_I32 || type->type == TYPE_I64;
}

bool type_is_unsigned_integral(Type const * type) {
	return type->type == TYPE_U8 || type->type == TYPE_U16 || type->type == TYPE_U32 || type->type == TYPE_U64;
}

bool type_is_integral(Type const * type) {
	return type_is_signed_integral(type) || type_is_unsigned_integral(type);
}	

bool type_is_boolean(Type const * type) {
	return type->type == TYPE_BOOL;
}

bool type_is_pointer(Type const * type) {
	return type->type == TYPE_POINTER;
}

bool is_void_pointer(Type const * type) {
	return type_is_pointer(type) && type_is_void(type->ptr);
}


bool types_equal(Type const * a, Type const * b) {
	if (a->type != b->type) return false;
}

bool types_unifiable(Type const * a, Type const * b) {
	if (type_is_integral(a) && type_is_integral(b)) return true;

	if (a->type == TYPE_POINTER && b->type == TYPE_POINTER) {
		// If either type is a void star and the other is a pointer as well, the types are considered equal
		if (a->ptr->type == TYPE_VOID || b->ptr->type == TYPE_VOID) return true;

		return types_unifiable(a->ptr, b->ptr);
	}

	return a->type == b->type;
}

Type * types_unify(Type const * a, Type const * b) {
	if (types_equal(a, b)) return a;

	if (type_is_integral(a) && type_is_integral(b)) {
		int size_a = type_get_size(a);
		int size_b = type_get_size(b);

		return size_a >= size_b ? a : b;
	}

	if (type_is_pointer(a) && type_is_pointer(b)) {
		if (is_void_pointer(a)) return b;
		if (is_void_pointer(b)) return a;
	}

	char str_type_a[128];
	char str_type_b[128];

	type_to_string(a, str_type_a, sizeof(str_type_a));
	type_to_string(b, str_type_b, sizeof(str_type_b));

	printf("TYPE ERROR: Unable to unify types '%s' and '%s'!", str_type_a, str_type_b);
	abort();
}

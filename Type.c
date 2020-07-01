#include "Type.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "Scope.h"

Type make_type_void() { return (Type){ TYPE_VOID, 0 }; }

Type make_type_i8 () { return (Type){ TYPE_I8,  0 }; }
Type make_type_i16() { return (Type){ TYPE_I16, 0 }; }
Type make_type_i32() { return (Type){ TYPE_I32, 0 }; }
Type make_type_i64() { return (Type){ TYPE_I64, 0 }; }

Type make_type_u8 () { return (Type){ TYPE_U8,  0 }; }
Type make_type_u16() { return (Type){ TYPE_U16, 0 }; }
Type make_type_u32() { return (Type){ TYPE_U32, 0 }; }
Type make_type_u64() { return (Type){ TYPE_U64, 0 }; }

Type make_type_bool() { return (Type){ TYPE_BOOL, 0 }; }

Type make_type_pointer(Type const * type) {
	Type ptr_type;
	ptr_type.type      = type->type;
	ptr_type.ptr_level = type->ptr_level + 1;

	return ptr_type;
}

Type make_type_deref(Type const * type) {
	assert(type_is_pointer(type));

	Type deref_type;
	deref_type.type      = type->type;
	deref_type.ptr_level = type->ptr_level - 1;

	return deref_type;
}

Type make_type_str() {
	return (Type){ TYPE_U8, 1 };
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

		case TYPE_STRUCT: sprintf_s(string, string_size, "%s", type->struct_name); break;

		default: abort();
	}

	int ptr_level = type->ptr_level;

	while (ptr_level > 0) {
		strcat_s(string, string_size, "*");

		ptr_level--;
	}
}

int type_get_size(Type const * type, Scope * scope) {
	if (type_is_pointer(type)) return 8;

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

		case TYPE_STRUCT: return scope_get_struct_def(scope, type->struct_name)->members->size;

		default: abort();
	}
}

int type_get_align(Type const * type, Scope * scope) {
	if (type_is_pointer(type)) return 8;

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

		case TYPE_STRUCT: return scope_get_struct_def(scope, type->struct_name)->members->align;

		default: abort();
	}
}

void align(int * address, int alignment) {
	int remainder = *address & (alignment - 1);
	if (remainder == 0) return;
	
	*address += alignment - remainder;
}


bool type_is_void(Type const * type) {
	return !type_is_pointer(type) && type->type == TYPE_VOID;
}

bool type_is_signed_integral(Type const * type) {
	return !type_is_pointer(type) && (type->type == TYPE_I8 || type->type == TYPE_I16 || type->type == TYPE_I32 || type->type == TYPE_I64);
}

bool type_is_unsigned_integral(Type const * type) {
	return !type_is_pointer(type) && (type->type == TYPE_U8 || type->type == TYPE_U16 || type->type == TYPE_U32 || type->type == TYPE_U64);
}

bool type_is_integral(Type const * type) {
	return type_is_signed_integral(type) || type_is_unsigned_integral(type);
}	

bool type_is_boolean(Type const * type) {
	return !type_is_pointer(type) && type->type == TYPE_BOOL;
}

bool type_is_pointer(Type const * type) {
	return type->ptr_level > 0;
}

bool type_is_struct(Type const * type) {
	return !type_is_pointer(type) && type->type == TYPE_STRUCT;
}

bool type_is_void_pointer(Type const * type) {
	return type_is_pointer(type) && type->type == TYPE_VOID;
}

bool type_is_primitive(Type const * type) {
	return type_is_void(type) || type_is_integral(type) || type_is_boolean(type) || type_is_pointer(type);
}


bool types_equal(Type const * a, Type const * b) {
	if (a->type != b->type) return false;

	if (type_is_pointer(a)) {
		return a->ptr_level == b->ptr_level;
	}

	if (type_is_struct(a)) {
		return strcmp(a->struct_name, b->struct_name) == 0;
	}

	return true;
}

bool types_unifiable(Type const * a, Type const * b) {
	if (type_is_integral(a) && type_is_integral(b)) return true;

	if (type_is_pointer(a) && type_is_pointer(b)) {
		// If either type is a void star and the other is a pointer as well, the types are considered equal
		if ((a->type == TYPE_VOID || b->type == TYPE_VOID) && a->ptr_level == b->ptr_level) return true;

		return types_equal(a, b);
	}

	if (type_is_struct(a) && type_is_struct(b)) {
		return strcmp(a->struct_name, b->struct_name) == 0;
	}

	return a->type == b->type;
}

Type types_unify(Type const * a, Type const * b, Scope * scope) {
	if (types_equal(a, b)) return *a;

	if (type_is_integral(a) && type_is_integral(b)) {
		int size_a = type_get_size(a, scope);
		int size_b = type_get_size(b, scope);

		return size_a >= size_b ? *a : *b;
	}

	if (type_is_pointer(a) && type_is_pointer(b)) {
		if (type_is_void_pointer(a)) return *b;
		if (type_is_void_pointer(b)) return *a;
	}

	char str_type_a[128];
	char str_type_b[128];

	type_to_string(a, str_type_a, sizeof(str_type_a));
	type_to_string(b, str_type_b, sizeof(str_type_b));

	printf("TYPE ERROR: Unable to unify types '%s' and '%s'!", str_type_a, str_type_b);
	abort();
}

#include "Type.h"

#include <string.h>

#include <stdio.h>
#include <stdlib.h>

static Type type_void = { TYPE_VOID, NULL };
static Type type_int  = { TYPE_INT,  NULL };
static Type type_bool = { TYPE_BOOL, NULL };
static Type type_char = { TYPE_CHAR, NULL };

Type * make_type_void() {
	return &type_void;
}

Type * make_type_int() {
	return &type_int;
}

Type * make_type_bool() {
	return &type_bool;
}

Type * make_type_char() {
	return &type_char;
}

Type * make_type_pointer(Type const * type) {
	Type * ptr_type = malloc(sizeof(Type));
	ptr_type->type = TYPE_POINTER;
	ptr_type->ptr  = type;

	return ptr_type;
}

void type_to_string(Type const * type, char * string, int string_size) {
	switch (type->type) {
		case TYPE_VOID: sprintf_s(string, string_size, "void"); break;
		case TYPE_INT:  sprintf_s(string, string_size, "int");  break;
		case TYPE_BOOL: sprintf_s(string, string_size, "bool"); break;
		case TYPE_CHAR: sprintf_s(string, string_size, "char"); break;

		case TYPE_POINTER: {
			type_to_string(type->ptr, string, string_size);
			strcat_s(string, string_size, " *");

			break;
		}

		default: abort();
	}
}

int type_get_size(Type const * type) {
	//return 8; // TEMP

	switch (type->type) {
		case TYPE_INT:     return 8; // 64 bit
		case TYPE_BOOL:    return 1;
		case TYPE_CHAR:    return 1;
		case TYPE_POINTER: return 8;

		default: abort();
	}
}

bool type_is_void(Type const * type) {
	return type->type == TYPE_VOID;
}

bool type_is_integral(Type const * type) {
	return type->type == TYPE_INT || type->type == TYPE_CHAR;
}

bool type_is_boolean(Type const * type) {
	return type->type == TYPE_BOOL;
}

bool type_is_pointer (Type const * type) {
	return type->type == TYPE_POINTER;
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
	if (types_unifiable(a, b)) return a;

	if (type_is_integral(a) && type_is_integral(b)) {
		int size_a = type_get_size(a);
		int size_b = type_get_size(b);

		return size_a >= size_b ? a : b;
	}

	char str_type_a[128];
	char str_type_b[128];

	type_to_string(a, str_type_a, sizeof(str_type_a));
	type_to_string(b, str_type_b, sizeof(str_type_b));

	printf("TYPE ERROR: Unable to unify types '%s' and '%s'!", str_type_a, str_type_b);
	abort();
}

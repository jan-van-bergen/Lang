#include "Type.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "AST.h"
#include "Scope.h"

#include "Util.h"
#include "Error.h"

// Types are allocated in Blocks that form a linked list
// This allows for stable pointers to types

#define TYPE_TABLE_BLOCK_SIZE 128

typedef struct Type_Block {
	Type table[TYPE_TABLE_BLOCK_SIZE];
	int  table_len;

	struct Type_Block * next;
} Type_Block;

static Type_Block * type_block_first;
static Type_Block * type_block_curr;

void type_table_init() {
	type_block_first = malloc(sizeof(Type_Block));
	type_block_first->next = NULL;

	// Fill with primitive types
	type_block_first->table[TYPE_VOID] = (Type){ TYPE_VOID, 0 };

	type_block_first->table[TYPE_I8 ] = (Type){ TYPE_I8,  0 };
	type_block_first->table[TYPE_I16] = (Type){ TYPE_I16, 0 };
	type_block_first->table[TYPE_I32] = (Type){ TYPE_I32, 0 };
	type_block_first->table[TYPE_I64] = (Type){ TYPE_I64, 0 };

	type_block_first->table[TYPE_U8 ] = (Type){ TYPE_U8,  0 };
	type_block_first->table[TYPE_U16] = (Type){ TYPE_U16, 0 };
	type_block_first->table[TYPE_U32] = (Type){ TYPE_U32, 0 };
	type_block_first->table[TYPE_U64] = (Type){ TYPE_U64, 0 };

	type_block_first->table[TYPE_F32] = (Type){ TYPE_F32, 0 };
	type_block_first->table[TYPE_F64] = (Type){ TYPE_F64, 0 };

	type_block_first->table[TYPE_BOOL] = (Type){ TYPE_BOOL, 0 };

	type_block_first->table_len = TYPE_POINTER;

	type_block_curr = type_block_first;
}

static void free_type_block(Type_Block * block) {
	if (block->next) free_type_block(block->next);

	free(block);
}

void type_table_free() {
	free_type_block(type_block_first);

	type_block_curr = NULL;
}

Type * type_table_new_type() {
	if (type_block_curr->table_len == TYPE_TABLE_BLOCK_SIZE) {
		Type_Block * block = malloc(sizeof(Type_Block));
		block->table_len = 0;
		block->next = NULL;

		type_block_curr->next = block;
		type_block_curr       = block;
	}

	return type_block_curr->table + type_block_curr->table_len++;
}


Type const * make_type_void() { return &type_block_first->table[TYPE_VOID]; }

Type const * make_type_i8 () { return &type_block_first->table[TYPE_I8 ]; }
Type const * make_type_i16() { return &type_block_first->table[TYPE_I16]; }
Type const * make_type_i32() { return &type_block_first->table[TYPE_I32]; }
Type const * make_type_i64() { return &type_block_first->table[TYPE_I64]; }

Type const * make_type_u8 () { return &type_block_first->table[TYPE_U8 ]; }
Type const * make_type_u16() { return &type_block_first->table[TYPE_U16]; }
Type const * make_type_u32() { return &type_block_first->table[TYPE_U32]; }
Type const * make_type_u64() { return &type_block_first->table[TYPE_U64]; }

Type const * make_type_f32() { return &type_block_first->table[TYPE_F32]; }
Type const * make_type_f64() { return &type_block_first->table[TYPE_F64]; }

Type const * make_type_bool() { return &type_block_first->table[TYPE_BOOL]; }


Type const * make_type_array(Type const * base_type, int size) {
	Type * ptr_type = type_table_new_type();
	ptr_type->type = TYPE_ARRAY;
	ptr_type->base = base_type;
	ptr_type->array_size = size;

	return ptr_type;
}

Type const * make_type_pointer(Type const * base_type) {
	Type * ptr_type = type_table_new_type();
	ptr_type->type = TYPE_POINTER;
	ptr_type->base = base_type;

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

		case TYPE_F32: sprintf_s(string, string_size, "f32"); break;
		case TYPE_F64: sprintf_s(string, string_size, "f64"); break;

		case TYPE_BOOL: sprintf_s(string, string_size, "bool"); break;

		case TYPE_POINTER: {
			type_to_string(type->base, string, string_size);
			strcat_s(string, string_size, "*");

			break;
		}

		case TYPE_ARRAY: {
			type_to_string(type->base, string, string_size);

			int str_len = strlen(string);
			sprintf_s(string + str_len, string_size - str_len, "[%i]", type->array_size);

			break;
		}

		case TYPE_STRUCT: sprintf_s(string, string_size, "%s", type->struct_name); break;

		default: error(ERROR_TYPECHECK);
	}
}

int type_get_size(Type const * type, Scope * scope) {
	switch (type->type) {
		case TYPE_VOID: {
			printf("ERROR: Cannot get size of void type!\n");
			error(ERROR_TYPECHECK); // Invalid!
		}

		case TYPE_I8:  return 1;
		case TYPE_I16: return 2;
		case TYPE_I32: return 4;
		case TYPE_I64: return 8;

		case TYPE_U8:  return 1;
		case TYPE_U16: return 2;
		case TYPE_U32: return 4;
		case TYPE_U64: return 8;

		case TYPE_F32: return 4;
		case TYPE_F64: return 8;

		case TYPE_BOOL: return 1;

		case TYPE_POINTER: return 8;

		case TYPE_ARRAY: return type_get_size(type->base, scope) * type->array_size;

		case TYPE_STRUCT: return scope_get_struct_def(scope, type->struct_name)->members->size;

		default: error(ERROR_UNKNOWN);
	}
}

int type_get_align(Type const * type, Scope * scope) {
	switch (type->type) {
		case TYPE_VOID: {
			printf("ERROR: Cannot get alignment of void type!\n");
			error(ERROR_TYPECHECK); // Invalid!
		}
		case TYPE_I8:  return 1;
		case TYPE_I16: return 2;
		case TYPE_I32: return 4;
		case TYPE_I64: return 8;

		case TYPE_U8:  return 1;
		case TYPE_U16: return 2;
		case TYPE_U32: return 4;
		case TYPE_U64: return 8;

		case TYPE_F32: return 4;
		case TYPE_F64: return 8;

		case TYPE_BOOL: return 1;

		case TYPE_POINTER: return 8;

		case TYPE_ARRAY: return type_get_align(type->base, scope);

		case TYPE_STRUCT: return scope_get_struct_def(scope, type->struct_name)->members->align;

		default: error(ERROR_UNKNOWN);
	}
}

void align(int * address, int alignment) {
	int remainder = *address & (alignment - 1);
	if (remainder == 0) return;
	
	*address += alignment - remainder;
}


bool type_is_void(Type const * type) { return type->type == TYPE_VOID; }

bool type_is_i8 (Type const * type) { return type->type == TYPE_I8;  }
bool type_is_i16(Type const * type) { return type->type == TYPE_I16; }
bool type_is_i32(Type const * type) { return type->type == TYPE_I32; }
bool type_is_i64(Type const * type) { return type->type == TYPE_I64; }

bool type_is_u8 (Type const * type) { return type->type == TYPE_U8;  }
bool type_is_u16(Type const * type) { return type->type == TYPE_U16; }
bool type_is_u32(Type const * type) { return type->type == TYPE_U32; }
bool type_is_u64(Type const * type) { return type->type == TYPE_U64; }

bool type_is_f32(Type const * type) { return type->type == TYPE_F32; }
bool type_is_f64(Type const * type) { return type->type == TYPE_F64; }

bool type_is_bool(Type const * type) { return type->type == TYPE_BOOL; }

bool type_is_array(Type const * type) { return type->type == TYPE_ARRAY; }

bool type_is_pointer(Type const * type) { return type->type == TYPE_POINTER; }

bool type_is_struct(Type const * type) { return type->type == TYPE_STRUCT; }

bool type_is_integral_signed(Type const * type) {
	return type->type == TYPE_I8 || type->type == TYPE_I16 || type->type == TYPE_I32 || type->type == TYPE_I64;
}

bool type_is_integral_unsigned(Type const * type) {
	return type->type == TYPE_U8 || type->type == TYPE_U16 || type->type == TYPE_U32 || type->type == TYPE_U64;
}

bool type_is_integral(Type const * type) {
	return type_is_integral_signed(type) || type_is_integral_unsigned(type);
}	

bool type_is_float(Type const * type) {
	return type->type == TYPE_F32 || type->type == TYPE_F64;
}

bool type_is_arithmetic(Type const * type) {
	return type_is_integral(type) || type_is_float(type);
}

bool type_is_void_pointer(Type const * type) {
	return type_is_pointer(type) && type->base->type == TYPE_VOID;
}

bool type_is_primitive(Type const * type) {
	return type_is_void(type) || type_is_arithmetic(type) || type_is_bool(type) || type_is_pointer(type);
}


bool types_equal(Type const * a, Type const * b) {
	if (a->type != b->type) return false;

	if (type_is_pointer(a)) {
		return types_equal(a->base, b->base);
	}

	if (type_is_array(a)) {
		return types_equal(a->base, b->base) && a->array_size == b->array_size;
	}

	if (type_is_struct(a)) {
		return strcmp(a->struct_name, b->struct_name) == 0;
	}

	return true;
}

bool types_unifiable(Type const * a, Type const * b) {
	if (type_is_integral(a) && type_is_integral(b)) return true;

	// if (type_is_float(a) && type_is_float(b)) return true;

	if (type_is_pointer(a) && type_is_pointer(b)) {
		// If either type is a void star and the other is a pointer as well, the types are considered equal
		if (a->base->type == TYPE_VOID || b->base->type == TYPE_VOID) return true;

		return types_equal(a->base, b->base);
	}

	// Arrays can decay to pointers
	if (type_is_pointer(a) && type_is_array(b)) return types_equal(a->base, b->base) || type_is_void_pointer(a);
	if (type_is_pointer(b) && type_is_array(a)) return types_equal(a->base, b->base) || type_is_void_pointer(b);

	if (type_is_struct(a) && type_is_struct(b)) {
		return strcmp(a->struct_name, b->struct_name) == 0;
	}

	return a->type == b->type;
}

Type const * types_unify(Type const * a, Type const * b, Scope * scope) {
	if (types_equal(a, b)) return a;

	if (type_is_integral(a) && type_is_integral(b)) {
		int size_a = type_get_size(a, scope);
		int size_b = type_get_size(b, scope);

		return size_a >= size_b ? a : b;
	}

	// Decay array to pointer
	if (type_is_pointer(a) && type_is_array(b)) return a;
	if (type_is_pointer(b) && type_is_array(a)) return b;

	if (type_is_pointer(a) && type_is_pointer(b)) {
		if (type_is_void_pointer(a)) return b;
		if (type_is_void_pointer(b)) return a;
	}

	char str_type_a[128];
	char str_type_b[128];

	type_to_string(a, str_type_a, sizeof(str_type_a));
	type_to_string(b, str_type_b, sizeof(str_type_b));

	printf("TYPE ERROR: Unable to unify types '%s' and '%s'!", str_type_a, str_type_b);
	error(ERROR_TYPECHECK);
}

Type const * type_dereference(Type const * type) {
	if (type->type != TYPE_POINTER) {
		char str_type[128];
		type_to_string(type, str_type, sizeof(str_type));

		printf("ERROR: Attempt to dereference non-pointer type '%s'!\n", str_type);
		error(ERROR_TYPECHECK);
	}

	return type->base;
}

Type const * type_infer(AST_Expression const * expr, Scope const * scope) {
	switch (expr->type) {
		case AST_EXPRESSION_CONST: {
			Token_Type literal_type = expr->expr_const.token.type;

			switch (literal_type) {
				case TOKEN_LITERAL_INT: {
					if (expr->expr_const.token.sign) {
						long long value = expr->expr_const.token.value_int;

						if (value >= INT_MIN && value <= INT_MAX) {
							return make_type_i32();
						} else {
							return make_type_i64();
						}
					} else {
						unsigned long long value = expr->expr_const.token.value_int;

						if (value <= UINT_MAX) {
							return make_type_u32();
						} else {
							return make_type_u64();
						}
					}
				}

				case TOKEN_LITERAL_F32: return make_type_f32();
				case TOKEN_LITERAL_F64: return make_type_f64();

				case TOKEN_LITERAL_BOOL: return make_type_bool();
				case TOKEN_LITERAL_CHAR: return make_type_u8();

				case TOKEN_LITERAL_STRING: return make_type_pointer(make_type_u8());

				case TOKEN_KEYWORD_NULL: return make_type_pointer(make_type_void());
					
				default: error(ERROR_UNKNOWN);
			}
		}

		case AST_EXPRESSION_VAR: return scope_get_variable(scope, expr->expr_var.name)->type;

		case AST_EXPRESSION_OPERATOR_BIN: {
			Type const * type_left  = type_infer(expr->expr_op_bin.expr_left,  scope);
			Type const * type_right = type_infer(expr->expr_op_bin.expr_right, scope);

			if (!types_unifiable(type_left, type_right)) break;

			return types_unify(type_left, type_right, scope);
		}

		case AST_EXPRESSION_OPERATOR_PRE:  {
			Type const * type_inner = type_infer(expr->expr_op_pre.expr, scope);

			switch (expr->expr_op_pre.token.type) {
				case TOKEN_OPERATOR_BITWISE_AND: return make_type_pointer(type_inner); // Address of  (&)
				case TOKEN_OPERATOR_MULTIPLY:    return type_dereference (type_inner); // Dereference (*)

				case TOKEN_OPERATOR_INC:
				case TOKEN_OPERATOR_DEC:
				case TOKEN_OPERATOR_PLUS: return type_inner;

				case TOKEN_OPERATOR_MINUS: {
					if (type_is_integral_unsigned(type_inner)) { // Turn unsigned integral type into its signed counterpart
						switch (type_inner->type) {
							case TYPE_U8:  return make_type_i8();
							case TYPE_U16: return make_type_i16();
							case TYPE_U32: return make_type_i32();
							case TYPE_U64: return make_type_i64();

							default: error(ERROR_UNKNOWN);
						}
					}
					return type_inner;
				}
			}
		}

		case AST_EXPRESSION_OPERATOR_POST: return type_infer(expr->expr_op_post.expr, scope);

		case AST_EXPRESSION_CAST: return expr->expr_cast.new_type;

		case AST_EXPRESSION_CALL_FUNC: return scope_get_function_def(scope, expr->expr_call.function_name)->return_type;

		case AST_EXPRESSION_ARRAY_ACCESS: {
			Type const * type = type_infer(expr->expr_array_access.expr_array, scope);

			if (!type_is_array(type) && !type_is_pointer(type)) {
				error(ERROR_TYPECHECK);
			}

			return type->base;
		}

		case AST_EXPRESSION_STRUCT_MEMBER: {
			Type const * struct_type = type_infer(expr->expr_struct_member.expr, scope);

			char const * struct_name = NULL;

			if (type_is_struct(struct_type)) {
				struct_name = struct_type->struct_name;
			} else if (type_is_pointer(struct_type) && type_is_struct(struct_type->base)) {
				struct_name = struct_type->base->struct_name;
			} else {
				error(ERROR_TYPECHECK);
			}

			Struct_Def * struct_def = scope_get_struct_def(scope, struct_name);
			Variable   * var_member = scope_get_variable(struct_def->member_scope, expr->expr_struct_member.member_name);

			return var_member->type;
		}
	}

	char str_expr[1024];
	ast_print_expression(expr, str_expr, sizeof(str_expr));

	printf("ERROR: Unable to infer type of exprssion '%s'!\n", str_expr);
	error(ERROR_TYPECHECK);
}

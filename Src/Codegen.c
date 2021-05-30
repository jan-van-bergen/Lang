#include "Codegen.h"

#include <assert.h>
#include <string.h>
#include <limits.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "Code_Emitter.h"

#include "Util.h"
#include "Error.h"

#define LABEL_STR_BUF_SIZE 64

static char const * const_f32_to_u64_name = "__const_f32_to_u64";
static char const * const_f64_to_u64_name = "__const_f64_to_u64";

static Result codegen_expression(Code_Emitter * emit, AST_Expression const * expr);
static void   codegen_statement (Code_Emitter * emit, AST_Statement  const * stat);

static Result codegen_expression_const(Code_Emitter * emit, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_CONST);

	Type const * type = type_infer(expr, emit->current_scope);

	Token_Type literal_type = expr->expr_const.token.type;
	switch (literal_type) {
		case TOKEN_LITERAL_STRING: {
			char str_lit_name[128]; sprintf_s(str_lit_name, sizeof(str_lit_name), "str_%i", emit->data_seg_len);

			Result result_reg = result_make_reg(type, register_alloc(emit));
			Result result_lit = result_make_global(type, str_lit_name);

			emit_string_literal(emit, str_lit_name, expr->expr_const.token.value_str);
			emit_lea(emit, &result_reg, &result_lit);

			return result_reg;
		}

		case TOKEN_LITERAL_BOOL:
		case TOKEN_LITERAL_CHAR: 
		case TOKEN_LITERAL_INT: {
			if (expr->expr_const.token.sign) {
				int64_t value = expr->expr_const.token.value_int;
				Result result_imm = result_make_i64(type, value);
				if (value > INT_MIN && value <= INT_MAX) {
					// Value fits in imm32, return as immediate
					return result_imm;
				} else {
					// Value does not fit in imm32, move to register
					Result result_reg = result_make_reg(type, register_alloc(emit));
					emit_mov(emit, &result_reg, &result_imm);
					return result_reg;
				}
			} else {
				uint64_t value = expr->expr_const.token.value_int;
				Result result_imm = result_make_u64(type, value);
				if (value <= UINT_MAX) {
					// Value fits in imm32, return as immediate
					return result_imm;
				} else {
					// Value does not fit in imm32, move to register
					Result result_reg = result_make_reg(type, register_alloc(emit));
					emit_mov(emit, &result_reg, &result_imm);
					return result_reg;
				}
			}
		}

		case TOKEN_LITERAL_F32: {
			char flt_lit_name[128]; sprintf_s(flt_lit_name, sizeof(flt_lit_name), "flt_%i", emit->data_seg_len);

			Result result_reg = result_make_reg(type, register_alloc_float(emit));
			Result result_lit = result_make_global(type, flt_lit_name);
			
			emit_f32_literal(emit, flt_lit_name, expr->expr_const.token.value_float);
			emit_mov(emit, &result_reg, &result_lit);

			return result_reg;
		}
		case TOKEN_LITERAL_F64: {
			char dbl_lit_name[128]; sprintf_s(dbl_lit_name, sizeof(dbl_lit_name), "dbl_%i", emit->data_seg_len);

			Result result_reg = result_make_reg(type, register_alloc_float(emit));
			Result result_lit = result_make_global(type, dbl_lit_name);
			
			emit_f64_literal(emit, dbl_lit_name, expr->expr_const.token.value_double);
			emit_mov(emit, &result_reg, &result_lit);

			return result_reg;
		}

		case TOKEN_KEYWORD_NULL: return result_make_u64(type, 0);
	}
	error_internal();
}

static Result codegen_expression_var(Code_Emitter * emit, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_VAR);

	char     const * var_name = expr->expr_var.name;
	Variable const * var = scope_get_variable(emit->current_scope, var_name);

	bool by_address = flag_is_set(emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
	
	bool is_global_function = false;
	if (type_is_function(var->type)) {
		Function_Def * function_def = scope_get_function_def(emit->current_scope, var_name);
		is_global_function = function_def != NULL;
	}

	Result result;
	if (var->is_global || is_global_function) {
		result = result_make_global(var->type, var->name);
	} else {
		result = result_make_sib(var->type, RBP, 0, 0, var->offset);
	}

	bool is_global_ptr_to_data_segment =
		var->is_global &&
		type_is_pointer(var->type) &&
		(type_is_u8(var->type->base) || type_is_float(var->type->base));

	if (by_address || is_global_ptr_to_data_segment || type_is_array(var->type) || type_is_struct(var->type) || type_is_function(var->type)) {
		result.by_address = true;
	}

	if (type_is_float(var->type)) {
		result_ensure_in_register(emit, &result);
	}

	return result;
}

static Result codegen_expression_array_access(Code_Emitter * emit, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_ARRAY_ACCESS);

	bool return_by_address = emit->flags & EMIT_FLAG_EVAL_BY_ADDRESS; 
	
	AST_Expression * expr_array = expr->expr_array_access.expr_array;
	AST_Expression * expr_index = expr->expr_array_access.expr_index;

	Result result_array, result_index;

	bool array_by_address = type_is_array(type_infer(expr_array, emit->current_scope));
	flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

	// Traverse tallest subtree first
	if (expr_array->height > expr_index->height) {
		if (array_by_address) flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
		result_array = codegen_expression(emit, expr_array);
		if (array_by_address) flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

		result_index = codegen_expression(emit, expr_index);
	} else {
		result_index = codegen_expression(emit, expr_index);
		
		if (array_by_address) flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
		result_array = codegen_expression(emit, expr_array);
		if (array_by_address) flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
	}

	if (!type_is_pointer(result_array.type) && !type_is_array(result_array.type)) {
		char str_type[128];
		type_to_string(result_array.type, str_type, sizeof(str_type));

		type_error(emit, "Operator '[]' requires left operand to be a pointer or an array. Type was '%s'", str_type);
	}

	if (type_is_void_pointer(result_array.type)) {
		type_error(emit, "Operator '[]' cannot dereference 'void *'");
	}

	if (!type_is_integral(result_index.type)) {
		char str_type[128];
		type_to_string(result_index.type, str_type, sizeof(str_type));

		type_error(emit, "Operator '[]' requires right operand to be an integral type. Type was '%s'", str_type);
	}
	
	// We don't want a pointer to the pointer, we want the pointer itself
	if (result_is_indirect(&result_array) && type_is_pointer(result_array.type)) {
		result_array.by_address = false;
		result_array = result_deref(emit, &result_array, true);
	}

	Result result_size = result_make_u64(make_type_u64(), type_get_size(result_array.type->base, emit->current_scope));
	emit_mul(emit, &result_index, &result_size);
	emit_add(emit, &result_array, &result_index);
	
	result_free(emit, &result_size);

	result_array.type = result_array.type->base;

	// Check if we need to return by address or value
	if (return_by_address) {
		flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
	} else {
		// Only dereference primitives, structs and arrays are always returned by address
		if (type_is_primitive(result_array.type)) {
			result_array.by_address = false;
			result_array = result_deref(emit, &result_array, true);
		}
	}
	
	result_free(emit, &result_index);
	
	return result_array;
}

static Result codegen_expression_struct_member(Code_Emitter * emit, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_STRUCT_MEMBER);

	bool var_by_address = flag_is_set(emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

	// Evaluate LHS by address
	flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
	Result result = codegen_expression(emit, expr->expr_struct_member.expr);

	char const * struct_name = NULL;

	if (type_is_struct(result.type)) {
		struct_name = result.type->struct_name;
	} else if (type_is_pointer(result.type) && type_is_struct(result.type->base)) {
		struct_name = result.type->base->struct_name;

		result.by_address = false;
		result = result_deref(emit, &result, true);
	} else if (type_is_array(result.type) && strcmp(expr->expr_struct_member.member_name, "length") == 0) {
		result_free(emit, &result);
		result = result_make_u64(make_type_u64(), result.type->array_size);
		return result;
	} else {
		char str_type[128];
		type_to_string(result.type, str_type, sizeof(str_type));

		type_error(emit, "Operator '.' requires left operand to be a struct or pointer to struct. Type was '%s'", str_type);
	}

	// Lookup the struct member by name
	Struct_Def const * struct_def = scope_get_struct_def(emit->current_scope, struct_name);
	Variable   const * var_member = scope_get_variable(struct_def->member_scope, expr->expr_struct_member.member_name);

	// Take the address of the struct and add the offset of the member
	Result result_offset = result_make_u64(make_type_u64(), var_member->offset);
	emit_add(emit, &result, &result_offset);
	result_free(emit, &result_offset);

	result.type = var_member->type;

	// Check if we need to return by value
	if (!var_by_address) {
		flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

		// Only dereference primitives, structs and arrays are always returned by address
		if (type_is_primitive(result.type)) {
			result.by_address = false;
			result = result_deref(emit, &result, true);
		}
	}

	return result;
}

static Result codegen_expression_cast(Code_Emitter * emit, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_CAST);

	Result result = codegen_expression(emit, expr->expr_cast.expr);

	Type const * type_old = result.type;
	Type const * type_new = expr->expr_cast.new_type;

	if (result.form == RESULT_IMMEDIATE) {
		if (type_is_integral(type_old)) {
			if (type_is_integral(type_new) || type_is_bool(type_new) || type_is_pointer(type_new)) {
				// Integral -> integral
				return result_make_u64(type_new, result.u64);
			} else if (type_is_f32(type_new)) {
				// Integral -> float
				if (type_is_integral_signed(type_old)) {
					return result_make_f32(type_new, (float)result.i64);
				} else {
					return result_make_f32(type_new, (float)result.u64);
				}
			} else if (type_is_f64(type_new)) {
				// Integral -> double
				if (type_is_integral_signed(type_old)) {
					return result_make_f64(type_new, (double)result.i64);
				} else {
					return result_make_f64(type_new, (double)result.u64);
				}
			}
		} else if (type_is_f32(type_old)) {
			if (type_is_integral_signed(type_new)) {
				// Float -> signed integral
				return result_make_i64(type_new, result.f32);
			} else if (type_is_integral_unsigned(type_new) || type_is_bool(type_new)) {
				// Float -> unsigned integral
				return result_make_u64(type_new, result.f32);
			} else if (type_is_f32(type_new)) {
				// Float -> float
				return result;
			} else if (type_is_f64(type_new)) {
				// Float -> double
				return result_make_f64(type_new, (double)result.f32);
			}
		} else if (type_is_f64(type_old)) {
			if (type_is_integral_signed(type_new)) {
				// Double -> signed integral
				return result_make_i64(type_new, result.f64);
			} else if (type_is_integral_unsigned(type_new) || type_is_bool(type_new)) {
				// Double -> unsigned integral
				return result_make_u64(type_new, result.f64);
			} else if (type_is_f32(type_new)) {
				// Double -> float
				return result_make_f32(type_new, (float)result.f64);
			} else if (type_is_f64(type_new)) {
				// Double -> double
				return result;
			}
		}
		error_internal();
	}

	if (!type_is_pointer(type_old)) {
		result_ensure_in_register(emit, &result);
	}

	if (type_is_integral(type_old)) {
		if (type_is_f32(type_new)) {
			int reg = register_alloc_float(emit);
			
			if (type_is_u64(type_old)) {
				if (result.form == RESULT_IMMEDIATE) {
					return result_make_f32(type_new, (float)result.u64);
				}

				int tmp = register_alloc(emit);

				int label_cvt_unsigned = get_new_label(emit);
				int label_cvt_exit     = get_new_label(emit);

				assert(result.form == RESULT_REGISTER);
				emit_asm(emit, "test %s, %s\n", get_reg_name(result.reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "js L_ctv_unsigned_%i\n", label_cvt_unsigned);
				emit_asm(emit, "cvtsi2ss %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "jmp L_ctv_exit_%i\n", label_cvt_exit);
				emit_asm(emit, "L_ctv_unsigned_%i:\n", label_cvt_unsigned);
				emit_asm(emit, "mov %s, %s\n", get_reg_name(tmp, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "and %s, 1\n", get_reg_name(result.reg, 8));
				emit_asm(emit, "shr %s, 1\n", get_reg_name(tmp, 8));
				emit_asm(emit, "or %s, %s\n", get_reg_name(tmp, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "cvtsi2ss %s, %s\n", get_reg_name(reg, 8), get_reg_name(tmp, 8));
				emit_asm(emit, "addss %s, %s\n", get_reg_name(reg, 8), get_reg_name(reg, 8));
				emit_asm(emit, "L_ctv_exit_%i:\n", label_cvt_exit);

				register_free(emit, tmp);
			} else {
				emit_asm(emit, "cvtsi2sd %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
			}

			result_free(emit, &result);
			result.reg = reg;
		} else if (type_is_f64(type_new)) {
			int reg = register_alloc_float(emit);

			if (type_is_u64(type_old)) {
				int tmp = register_alloc(emit);

				int label_cvt_unsigned = get_new_label(emit);
				int label_cvt_exit     = get_new_label(emit);
				
				emit_asm(emit, "test %s, %s\n", get_reg_name(result.reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "js L_cvt_unsigned_%i\n", label_cvt_unsigned);
				emit_asm(emit, "cvtsi2sd %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "jmp L_cvt_exit_%i\n", label_cvt_exit);
				emit_asm(emit, "L_cvt_unsigned_%i:\n", label_cvt_unsigned);
				emit_asm(emit, "mov %s, %s\n", get_reg_name(tmp, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "and %s, 1\n", get_reg_name(result.reg, 8));
				emit_asm(emit, "shr %s, 1\n", get_reg_name(tmp, 8));
				emit_asm(emit, "or %s, %s\n", get_reg_name(tmp, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "cvtsi2sd %s, %s\n", get_reg_name(reg, 8), get_reg_name(tmp, 8));
				emit_asm(emit, "addsd %s, %s\n", get_reg_name(reg, 8), get_reg_name(reg, 8));
				emit_asm(emit, "L_cvt_exit_%i:\n", label_cvt_exit);

				register_free(emit, tmp);
			} else {
				emit_asm(emit, "cvtsi2sd %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
			}

			register_free(emit, result.reg);
			result.reg = reg;
		}
	} else if (type_is_f32(type_old)) {
		if (type_is_integral(type_new)) {
			int reg = register_alloc(emit);

			if (type_is_u64(type_new)) {
				int label_cvt_unsigned = get_new_label(emit);
				int label_cvt_exit     = get_new_label(emit);
				
				emit_asm(emit, "comiss %s, DWORD [REL %s]\n", get_reg_name(result.reg, 8), const_f32_to_u64_name);
				emit_asm(emit, "jnb L_cvt_unsigned_%i\n", label_cvt_unsigned);
				emit_asm(emit, "cvttss2si %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "jmp L_cvt_exit_%i\n", label_cvt_exit);
				emit_asm(emit, "L_cvt_unsigned_%i:\n", label_cvt_unsigned);
				emit_asm(emit, "subss %s, DWORD [REL %s]\n", get_reg_name(result.reg, 8), const_f32_to_u64_name);
				emit_asm(emit, "cvttss2si %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "btc %s, 63\n", get_reg_name(reg, 8));
				emit_asm(emit, "L_cvt_exit_%i:\n", label_cvt_exit);			
			} else {
				emit_asm(emit, "cvttss2si %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
			}

			register_free(emit, result.reg);
			result.reg = reg;
		} else if (type_is_f64(type_new)) {
			emit_asm(emit, "cvtss2sd %s, %s\n", get_reg_name(result.reg, 8), get_reg_name(result.reg, 8));
		}
	} else if (type_is_f64(type_old)) {
		if (type_is_integral(type_new)) {
			int reg = register_alloc(emit);

			if (type_is_u64(type_new)) {
				int label_cvt_unsigned = get_new_label(emit);
				int label_cvt_exit     = get_new_label(emit);
				
				emit_asm(emit, "comisd %s, QWORD [REL %s]\n", get_reg_name(result.reg, 8), const_f64_to_u64_name);
				emit_asm(emit, "jnb L_cvt_unsigned_%i\n", label_cvt_unsigned);
				emit_asm(emit, "cvttsd2si %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "jmp L_cvt_exit_%i\n", label_cvt_exit);
				emit_asm(emit, "L_cvt_unsigned_%i:\n", label_cvt_unsigned);
				emit_asm(emit, "subsd %s, QWORD [REL %s]\n", get_reg_name(result.reg, 8), const_f64_to_u64_name);
				emit_asm(emit, "cvttsd2si %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
				emit_asm(emit, "btc %s, 63\n", get_reg_name(reg, 8));
				emit_asm(emit, "L_cvt_exit_%i:\n", label_cvt_exit);			
			} else {
				emit_asm(emit, "cvttsd2si %s, %s\n", get_reg_name(reg, 8), get_reg_name(result.reg, 8));
			}

			result_free(emit, &result);
			result.reg = reg;
		} else if (type_is_f32(type_new)) {
			emit_asm(emit, "cvtsd2ss %s, %s\n", get_reg_name(result.reg, 8), get_reg_name(result.reg, 8));
		}
	}

	result.type = type_new;

	return result;
}

static Result codegen_expression_sizeof(Code_Emitter * emit, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_SIZEOF);

	Type const * type = expr->expr_sizeof.type;
	int size = type_get_size(type, emit->current_scope);
	
	return result_make_u64(make_type_u32(), size);
}

static Result codegen_expression_op_bin(Code_Emitter * emit, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_BIN);

	Operator_Bin operator = expr->expr_op_bin.operator;
		
	AST_Expression const * expr_left  = expr->expr_op_bin.expr_left;
	AST_Expression const * expr_right = expr->expr_op_bin.expr_right;
	
	Result result_left, result_right;

	// Assignment operator is handled separately, because it needs the lhs by address
	if (operator == OPERATOR_BIN_ASSIGN) {
		if (!ast_is_lvalue(expr_left)) {
			char str_expr_left[512]; ast_print_expression(expr_left, str_expr_left, sizeof(str_expr_left));
			error(ERROR_CODEGEN, "Left hand operand of assignment '%s' must be an l-value!\n", str_expr_left);
		}

		// Traverse tallest subtree first
		if (expr_left->height > expr_right->height) {
			// Evaluate lhs by address
			flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
			result_left  = codegen_expression(emit, expr_left);
			flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

			result_right = codegen_expression(emit, expr_right);
		} else {
			result_right = codegen_expression(emit, expr_right);

			// Evaluate lhs by address
			flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
			result_left = codegen_expression(emit, expr_left);
			flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
		}

		if (result_left.form == RESULT_IMMEDIATE) {
			char str_expr_left[512]; ast_print_expression(expr_left, str_expr_left, sizeof(str_expr_left));
			error(ERROR_CODEGEN, "Cannot assign left hand operand '%s'!\n", str_expr_left);
		}

		if (type_is_void(result_right.type)) {
			type_error(emit, "Cannot assign with a right hand side of void type!");
		}

		int type_size_left  = type_get_size(result_left .type, emit->current_scope);
		int type_size_right = type_get_size(result_right.type, emit->current_scope);

		if (!types_unifiable(result_left.type, result_right.type)) {
			char str_type_left [128]; type_to_string(result_left .type, str_type_left,  sizeof(str_type_left));
			char str_type_right[128]; type_to_string(result_right.type, str_type_right, sizeof(str_type_right));

			type_error(emit, "Cannot assign instance of type '%s' a value of type '%s'", str_type_left, str_type_right);
		} else if (type_is_primitive(result_left .type) && type_is_primitive(result_right.type) && type_size_right > type_size_left) {
			char str_type_left [128]; type_to_string(result_left .type, str_type_left,  sizeof(str_type_left));
			char str_type_right[128]; type_to_string(result_right.type, str_type_right, sizeof(str_type_right));

			type_error(emit, "Implicit narrowing conversion from type '%s' to '%s' is not allowed. Explicit cast required", str_type_right, str_type_left);
		}

		if (type_is_struct(result_left.type)) {
			int struct_size = type_size_left;		
			if (struct_size <= 8) {
				// Use mov to copy by value if size fits in 8 bytes
				result_right.by_address = false;
				emit_mov_indirect(emit, &result_left, &result_right);
			} else {
				Result result_rdi = result_make_reg(result_left .type, RDI);
				Result result_rsi = result_make_reg(result_right.type, RSI);
				Result result_rcx = result_make_reg(make_type_u64(), RCX);
				Result result_len = result_make_u64(make_type_u64(), struct_size);

				emit_mov(emit, &result_rdi, &result_left);
				emit_mov(emit, &result_rsi, &result_right);
				emit_mov(emit, &result_rcx, &result_len);
				emit_rep_movsb(emit);

				result_free(emit, &result_len);
			}
		} else {
			emit_mov_indirect(emit, &result_left, &result_right);
		}

		result_free(emit, &result_right);

		result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);

		return result_left;
	}
	
	// Handle operators that require short-circuit evaluation separately
	if (operator == OPERATOR_BIN_LOGICAL_AND) {
		bool inside_condition = flag_is_set(emit->flags, EMIT_FLAG_INSIDE_CONDITION);
		if (expr_left->type == AST_EXPRESSION_OPERATOR_BIN && expr_left->expr_op_bin.operator == OPERATOR_BIN_LOGICAL_OR) {
			flag_unset(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);
		}

		result_left = codegen_expression(emit, expr_left);

		if (result_left.form == RESULT_IMMEDIATE) {
			if (result_left.u64 == 0) {
				return result_left;
			} else if (result_left.u64 == 1) {
				return codegen_expression(emit, expr_right);
			} else {
				error_internal();
			}
		}

		if (inside_condition) flag_set(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);

		if (flag_is_set(emit->flags, EMIT_FLAG_INSIDE_CONDITION)) {
			assert(emit->current_condition_label_true);
			assert(emit->current_condition_label_false);

			Condition_Code cc_left = condition_code_invert(result_get_condition_code(emit, &result_left));
			emit_jcc(emit, cc_left, emit->current_condition_label_false);	
			result_free(emit, &result_left);
			
			result_right = codegen_expression(emit, expr_right);
		} else {
			result_ensure_in_register(emit, &result_left);

			int  label = get_new_label(emit);
			char label_land_short[LABEL_STR_BUF_SIZE]; sprintf_s(label_land_short, sizeof(label_land_short), "L_land_short_%i", label);

			Condition_Code cc_left = condition_code_invert(result_get_condition_code(emit, &result_left));
			emit_jcc(emit, cc_left, label_land_short);	
			
			Register reg = result_left.reg;
			result_free(emit, &result_left);

			result_right = codegen_expression(emit, expr_right);
			result_ensure_in_given_register(emit, &result_right, reg);
			
			emit_label(emit, label_land_short);
		}

		if (!type_is_bool(result_left.type) || !type_is_bool(result_right.type)) {
			type_error(emit, "Operator '&&' requires two boolean operands");
		}
		
		return result_right;
	} else if (operator == TOKEN_OPERATOR_LOGICAL_OR) {
		bool inside_condition = flag_is_set(emit->flags, EMIT_FLAG_INSIDE_CONDITION);
		if (expr_left->type == AST_EXPRESSION_OPERATOR_BIN && expr_left->expr_op_bin.operator == OPERATOR_BIN_LOGICAL_AND) {
			flag_unset(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);
		}

		result_left = codegen_expression(emit, expr_left);
		
		if (result_left.form == RESULT_IMMEDIATE) {
			if (result_left.u64 == 1) {
				return result_left;
			} else if (result_left.u64 == 0) {
				return codegen_expression(emit, expr_right);
			} else {
				error_internal();
			}
		}
		
		if (inside_condition) flag_set(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);

		if (flag_is_set(emit->flags, EMIT_FLAG_INSIDE_CONDITION)) {
			assert(emit->current_condition_label_true);
			assert(emit->current_condition_label_false);

			Condition_Code cc_left = result_get_condition_code(emit, &result_left);
			emit_jcc(emit, cc_left, emit->current_condition_label_true);	
			result_free(emit, &result_left);
			
			result_right = codegen_expression(emit, expr_right);
		} else {
			result_ensure_in_register(emit, &result_left);
		
			int  label = get_new_label(emit);
			char label_lor_short[LABEL_STR_BUF_SIZE]; sprintf_s(label_lor_short, sizeof(label_lor_short), "L_lor_short_%i", label);

			Condition_Code cc_left = result_get_condition_code(emit, &result_left);
			emit_jcc(emit, cc_left, label_lor_short);	
			
			Register reg = result_left.reg;
			result_free(emit, &result_left);

			result_right = codegen_expression(emit, expr_right);
			result_ensure_in_given_register(emit, &result_right, reg);
			
			emit_label(emit, label_lor_short);
		}

		if (!type_is_bool(result_left.type) || !type_is_bool(result_right.type)) {
			type_error(emit, "Operator '||' requires two boolean operands");
		}
		
		return result_right;
	}

	// Traverse tallest subtree first
	if (expr_left->height > expr_right->height) {
		result_left  = codegen_expression(emit, expr_left);
		result_right = codegen_expression(emit, expr_right);
	} else {
		result_right = codegen_expression(emit, expr_right);
		result_left  = codegen_expression(emit, expr_left);
	}

	// Emit correct instructions based on operator type
	switch (operator) {
		case OPERATOR_BIN_PLUS: {		
			if (type_is_arithmetic(result_left.type) && type_is_arithmetic(result_right.type)) { // arithmetic + arithmetic --> arithmetic
				result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);
			} else if (type_is_pointer(result_left.type) && type_is_integral(result_right.type)) { // pointer + integral --> pointer
				Result result_size = result_make_u64(make_type_u64(), type_get_size(result_left.type->base, emit->current_scope));
				emit_mul(emit, &result_right, &result_size);
				result_free(emit, &result_size);
			} else if (type_is_array(result_left.type) && type_is_integral(result_right.type)) { // array + integral --> pointer
				result_left.type = make_type_pointer(result_left.type->base);
			} else {
				type_error(emit, "Left of operator '+' must be integral, float, array, or pointer type, right must be integral or float type!");
			}

			emit_add(emit, &result_left, &result_right);
			break;
		}

		case OPERATOR_BIN_MINUS: {
			if (type_is_arithmetic(result_left.type) && type_is_arithmetic(result_right.type)) { // arithmetic - arithmetic --> arithmetic
				result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);
			} else if (type_is_pointer(result_left.type) && type_is_integral(result_right.type)) { // pointer - integral --> pointer
				Result result_size = result_make_u64(make_type_u64(), type_get_size(result_left.type->base, emit->current_scope));
				emit_mul(emit, &result_right, &result_size);
				result_free(emit, &result_size);
			} else if (type_is_pointer(result_left.type) && type_is_pointer(result_right.type) && types_unifiable(result_left.type, result_right.type)) { // pointer - pointer --> integral
				result_left.type = make_type_i64();
			} else {
				type_error(emit, "Operator '-' cannot cannot have integral type on the left and pointer type on the right");
			}

			emit_sub(emit, &result_left, &result_right);
			break;
		}

		case OPERATOR_BIN_MULTIPLY: {
			if (type_is_integral(result_left.type) && type_is_integral(result_right.type)) {
				result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);
			} else if (type_is_f32(result_left.type) && type_is_f32(result_right.type)) {
				
			} else if (type_is_f64(result_left.type) && type_is_f64(result_right.type)) {
				
			} else {
				type_error(emit, "Operator '*' only works with integral or float types");
			}
			emit_mul(emit, &result_left, &result_right);
			break;
		}

		case OPERATOR_BIN_DIVIDE: {
			if (type_is_integral(result_left.type) && type_is_integral(result_right.type)) {
				result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);
			} else if (type_is_f32(result_left.type) && type_is_f32(result_right.type)) {
				
			} else if (type_is_f64(result_left.type) && type_is_f64(result_right.type)) {
				
			} else {
				type_error(emit, "Operator '/' only works with integral or float types");
			}
			emit_div(emit, &result_left, &result_right);
			break;
		}

		case OPERATOR_BIN_MODULO: {
			if (type_is_integral(result_left.type) && type_is_integral(result_right.type)) {
				result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);
			} else {
				type_error(emit, "Operator '%' only works with integral types");
			}
			emit_mod(emit, &result_left, &result_right);
			break;
		}

		case OPERATOR_BIN_SHIFT_LEFT: {
			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error(emit, "Operator '<<' requires two operands of integral type");
			}
			
			emit_shift_left(emit, &result_left, &result_right);
			break;
		}

		case OPERATOR_BIN_SHIFT_RIGHT: {
			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error(emit, "Operator '>>' requires two operands of integral type");
			}
			
			emit_shift_right(emit, &result_left, &result_right);
			break;
		}

		case OPERATOR_BIN_LT:
		case OPERATOR_BIN_LE:
		case OPERATOR_BIN_GT:
		case OPERATOR_BIN_GE:
		case OPERATOR_BIN_EQ:
		case OPERATOR_BIN_NE: {
			emit_cmp(emit, operator, &result_left, &result_right); 
			break;
		}

		case TOKEN_OPERATOR_BITWISE_AND: {
			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error(emit, "Operator '&' requires two operands of integral type");
			}
			result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);

			emit_and(emit, &result_left, &result_right);
			break;
		}

		case TOKEN_OPERATOR_BITWISE_XOR: {
			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error(emit, "Operator '^' requires two operands of integral type");
			}
			result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);
			
			emit_xor(emit, &result_left, &result_right);
			break;
		}

		case TOKEN_OPERATOR_BITWISE_OR: {
			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error(emit, "Operator '|' requires two operands of integral type");
			}
			result_left.type = types_unify(result_left.type, result_right.type, emit->current_scope);
			
			emit_or(emit, &result_left, &result_right);
			break;
		}

		default: error_internal();
	}

	result_free(emit, &result_right);

	return result_left;
}

static Result codegen_expression_op_pre(Code_Emitter * emit, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_PRE);

	AST_Expression * operand  = expr->expr_op_pre.expr;
	Operator_Pre     operator = expr->expr_op_pre.operator;

	Result result;

	// Check if this is a pointer operator
	if (operator == OPERATOR_PRE_ADDRESS_OF) {
		if (!ast_is_lvalue(operand)) {
			type_error(emit, "Operator '&' can only take address of an l-value");
		}

		bool by_address = flag_is_set(emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
		flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

		result = codegen_expression(emit, operand);
		result.type = make_type_pointer(result.type);

		if (!by_address) flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

		return result;
	} else if (operator == OPERATOR_PRE_DEREF) {
		bool by_address = flag_is_set(emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

		flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS); // Unset temporarily

		result = codegen_expression(emit, operand);
		
		if (!type_is_pointer(result.type)) {
			char str_type[128];
			type_to_string(result.type, str_type, sizeof(str_type));

			type_error(emit, "Cannot dereference non-pointer type '%s'", str_type);
		} else if (type_is_void_pointer(result.type)) {
			type_error(emit, "Cannot dereference 'void *'");
		}
		
		result_ensure_in_register(emit, &result);
		result.type = result.type->base; // Remove 1 level of indirection

		if (by_address) {
			flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS); // Reset if this flag was previously set
		} else {
			result.by_address = false;
			result = result_deref(emit, &result, true);
		}
		
		return result;
	}

	bool by_address = false;
	
	if (operator == OPERATOR_PRE_INC || operator == OPERATOR_PRE_DEC) {
		by_address = flag_is_set(emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
		flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
	}
	
	result = codegen_expression(emit, operand);

	if (!by_address) flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

	switch (operator) {
		case OPERATOR_PRE_INC: {
			Result result_address = result_make_reg(result.type, register_alloc(emit));
			emit_mov(emit,&result_address, &result);
			
			result.by_address = false;
			result = result_deref(emit, &result, false);

			int increase;
			if (type_is_integral(result.type)) {
				increase = 1;
			} else if (type_is_pointer(result.type)) {
				increase = type_get_size(result.type->base, emit->current_scope);
			} else {
				type_error(emit, "Operator '++' requires operand of integral or pointer type");
			}
			Result result_inc = result_make_u64(result.type, increase);
			emit_add(emit, &result, &result_inc);
			result_free(emit, &result_inc);

			emit_mov_indirect(emit, &result_address, &result);

			result_free(emit, &result_address);

			break;
		}

		case OPERATOR_PRE_DEC: {
			Result result_address = result_make_reg(result.type, register_alloc(emit));
			emit_mov(emit,&result_address, &result);
			
			result.by_address = false;
			result = result_deref(emit, &result, false);

			int decrease;
			if (type_is_integral(result.type)) {
				decrease = 1;
			} else if (type_is_pointer(result.type)) {
				decrease = type_get_size(result.type->base, emit->current_scope);
			} else {
				type_error(emit, "Operator '--' requires operand of integral or pointer type");
			}
			Result result_dec = result_make_i64(result.type, -decrease);
			emit_add(emit, &result, &result_dec);
			result_free(emit, &result_dec);

			emit_mov_indirect(emit, &result_address, &result);

			result_free(emit, &result_address);

			break;
		}

		case OPERATOR_PRE_PLUS: {
			// Unary plus is a no-op, no code is emitted

			if (!type_is_arithmetic(result.type)) {
				type_error(emit, "Operator '+' requires operand of integral or float type");
			}

			break;
		}
		case OPERATOR_PRE_MINUS: {
			emit_neg(emit, &result);

			if (!type_is_arithmetic(result.type)) {
				type_error(emit, "Operator '-' requires operand of integral or float type");
			}
			
			break;
		}

		case OPERATOR_PRE_BITWISE_NOT: {
			emit_not(emit, &result);

			if (!type_is_integral(result.type)) {
				type_error(emit, "Operator '~' requires operand of integral type");
			}

			break;
		}

		case OPERATOR_PRE_LOGICAL_NOT: {
			Result result_one     = result_make_i64(result.type,  1);
			Result result_neg_one = result_make_i64(result.type, -1);

			emit_xor(emit, &result, &result_neg_one);
			emit_and(emit, &result, &result_one);

			result_free(emit, &result_one);
			result_free(emit, &result_neg_one);

			if (!type_is_bool(result.type)) {
				type_error(emit, "Operator '!' requires operand of boolean type");
			}

			break;
		}

		default: error_internal();
	}

	return result;
}

static Result codegen_expression_op_post(Code_Emitter * emit, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_POST);

	AST_Expression * operand  = expr->expr_op_pre.expr;
	Operator_Post    operator = expr->expr_op_pre.operator;

	bool by_address = flag_is_set(emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

	// Evaluate operand by address
	flag_set(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);
	Result result = codegen_expression(emit, operand);

	if (!by_address) flag_unset(&emit->flags, EMIT_FLAG_EVAL_BY_ADDRESS);

	switch (operator) {
		case OPERATOR_POST_INC: {
			Result result_address;
			Result result_value = result_make_reg(result.type, register_alloc(emit));
			
			if (result_is_indirect(&result)) {
				result_address = result_make_reg(result.type, register_alloc(emit));
				emit_lea(emit, &result_address, &result);
			} else {
				result_address = result;
			}

			result.by_address = false;
			result = result_deref(emit, &result_address, false);

			int increase;

			if (type_is_integral(result.type)) {
				increase = 1;
			} else if (type_is_pointer(result.type)) {
				increase = type_get_size(result.type->base, emit->current_scope);
			} else {
				type_error(emit, "Operator '++' requires operand of integral or pointer type");
			}

			// Increase using effective address
			Result result_inc = result_make_sib(result.type, result.reg, 0, 0, increase);
			emit_lea(emit, &result_value, &result_inc);
			result_free(emit, &result_inc);

			emit_mov_indirect(emit, &result_address, &result_value);

			result_free(emit, &result_value);

			if (!type_is_integral(result.type) && !type_is_pointer(result.type)) {
				type_error(emit, "Operator '++' requires operand of integral or pointer type");
			}

			break;
		}

		case OPERATOR_POST_DEC: {
			Result result_address;
			Result result_value = result_make_reg(result.type, register_alloc(emit));
			
			if (result_is_indirect(&result)) {
				result_address = result_make_reg(result.type, register_alloc(emit));
				emit_lea(emit, &result_address, &result);
			} else {
				result_address = result;
			}

			result.by_address = false;
			result = result_deref(emit, &result_address, false);

			int decrease;

			if (type_is_integral(result.type)) {
				decrease = 1;
			} else if (type_is_pointer(result.type)) {
				decrease = type_get_size(result.type->base, emit->current_scope);
			} else {
				type_error(emit, "Operator '--' requires operand of integral or pointer type");
			}

			// Decrease using effective address
			Result result_inc = result_make_sib(result.type, result.reg, 0, 0, -decrease);
			emit_lea(emit, &result_value, &result_inc);
			result_free(emit, &result_inc);

			emit_mov_indirect(emit, &result_address, &result_value);

			result_free(emit, &result_value);

			if (!type_is_integral(result.type) && !type_is_pointer(result.type)) {
				type_error(emit, "Operator '--' requires operand of integral or pointer type");
			}

			break;
		}

		default: error_internal();
	}

	return result;
}

static Result codegen_expression_call_func(Code_Emitter * emit, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_CALL_FUNC);
	
	Type const * function_type = type_infer(expr->expr_call.expr_function, emit->current_scope);
	
	if (!type_is_function(function_type)) {
		type_error(emit, "Attempting to perform function call on non-function expression!");
	}

	Type const ** arg_types   = function_type->function.args;
	int           arg_count   = function_type->function.arg_count;
	Type const  * return_type = function_type->function.return_type;
	
	int call_arg_count = expr->expr_call.arg_count;
	
	if (call_arg_count != arg_count) {
		type_error(emit, "Incorrect number of arguments to function! Provided %i, needed %i", call_arg_count, arg_count);
	}

	// https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-160
	static Register const volatile_registers[] = { RAX, RBX, RCX, RDX, R8, R9, R10, R11, XMM0, XMM1, XMM2, XMM3, XMM4, XMM5 };

	regmask_t pushed_reg_mask  = 0;
	int       pushed_reg_count = 0;

	// Preserve volatile registers if needed
	for (int i = 0; i < ARRAY_COUNT(volatile_registers); i++) {
		Register reg = volatile_registers[i];
		if (register_is_reserved(emit, reg)) {
			flag_set(&pushed_reg_mask, 1u << reg);
			pushed_reg_count++;

			if (register_is_float(reg)) {
				emit_asm(emit, "sub rsp, 8 ; preserve XMM\n");
				emit_asm(emit, "movsd QWORD [rsp], %s\n", get_reg_name(reg, 8));
			} else {
				emit_asm(emit, "push %s ; preserve\n", get_reg_name(reg, 8));
			}
		}
	}

	// Get total size of arguments in bytes
	int   arg_size = 0;
	int * arg_offsets = _alloca(call_arg_count * sizeof(int));

	for (int i = 0; i < call_arg_count; i++) {
		Type const * arg_type = arg_types[i];

		arg_offsets[i] = arg_size;

		if (i < 4) {
			arg_size += 8;
		} else {
			int type_size  = type_get_size (arg_type, emit->current_scope);
			int type_align = type_get_align(arg_type, emit->current_scope);

			align(&arg_size, type_align);
			arg_size += type_size;
		}
	}

	if (arg_size < 32) {
		arg_size = 32; // Needs at least 32 bytes for shadow space
	} else {
		arg_size = (arg_size + 15) & ~15; // Round up to next multiple of 16 bytes to ensure stack alignment
	}

	if (pushed_reg_count & 1) arg_size += 8; // Pushed an uneven number of registers, add 8 to realign to multiple of 16

	// Reserve stack space for arguments
	emit_asm(emit, "sub rsp, %i ; reserve shadow space and %i arguments\n", arg_size, call_arg_count);

	bool any_arg_contains_function_call = false;

	for (int i = 0; i < call_arg_count; i++) {
		if (ast_contains(expr->expr_call.args[i].expr, AST_EXPRESSION_CALL_FUNC)) {
			any_arg_contains_function_call = true;
			break;
		}
	}

	// Evaluate arguments and put them into the right register / stack address
	// The first 4 arguments go in registers, the rest spill onto the stack
	for (int i = call_arg_count - 1; i >= 0; i--) {
		Result result_arg = codegen_expression(emit, expr->expr_call.args[i].expr);

		Type const * arg_type = arg_types[i];

		if (!types_unifiable(result_arg.type, arg_type)) {
			char str_type_given   [128]; type_to_string(result_arg.type, str_type_given,    sizeof(str_type_given));
			char str_type_expected[128]; type_to_string(arg_type,        str_type_expected, sizeof(str_type_expected));

			type_error(emit, "Argument %i in function call has incorrect type. Given type: '%s', expected type: '%s'", i + 1, str_type_given, str_type_expected);
		}
		result_arg.type = arg_type;

		// Move into shadow space if any argument contains another function call (thereby clobbering call registers)
		// Arguments after the first four allways go via the stack.
		bool via_stack = any_arg_contains_function_call || i >= 4;

		if (via_stack) {			
			Result result_rsp = result_make_sib(arg_type, RSP, 0, 0, arg_offsets[i]);
			emit_mov(emit, &result_rsp, &result_arg);

			result_free(emit, &result_rsp);
		} else {
			// Move into call register
			Result result_reg = result_make_reg(arg_types[i], get_call_register(i, type_is_float(arg_types[i])));
			emit_mov(emit, &result_reg, &result_arg);
		}

		result_free(emit, &result_arg);
	}
	
	if (any_arg_contains_function_call) {
		// Move first four saved arguments from stack into the call registers
		for (int i = 0; i < MIN(4, call_arg_count); i++) {
			Result result_reg = result_make_reg(arg_types[i], get_call_register(i, type_is_float(arg_types[i])));
			Result result_rsp = result_make_sib(arg_types[i], RSP, 0, 0, arg_offsets[i]);
			emit_mov(emit, &result_reg, &result_rsp);
		}
	}

	Function_Def * function_def = NULL;

	if (expr->expr_call.expr_function->type == AST_EXPRESSION_VAR) {
		char const * var_name = expr->expr_call.expr_function->expr_var.name;
		function_def = scope_get_function_def(emit->current_scope, var_name);
	}

	bool is_function_ptr = function_def == NULL;

	if (is_function_ptr) {
		Result result_function = codegen_expression(emit, expr->expr_call.expr_function);
		result_ensure_in_register(emit, &result_function);
		emit_asm(emit, "call [%s]\n", get_reg_name(result_function.reg, 8));
		register_free(emit, result_function.reg);
	} else {
		emit_asm(emit, "call %s\n", function_def->name);
	}

	emit_asm(emit, "add rsp, %i ; pop arguments\n", arg_size);

	// Restore volatile registers
	if (pushed_reg_count > 0) {
		for (int i = ARRAY_COUNT(volatile_registers) - 1; i >= 0; i--) {
			Register reg = volatile_registers[i];
			if (flag_is_set(pushed_reg_mask, 1u << reg)) {
				if (register_is_float(reg)) {
					emit_asm(emit, "movsd %s, QWORD [rsp] ; restore XMM\n", get_reg_name(reg, 8));
					emit_asm(emit, "add rsp, 8\n");
				} else {
					emit_asm(emit, "pop %s ; restore\n", get_reg_name(reg, 8));
				}
			}
		}
	}

	if (!type_is_void(return_type)) {
		Register reg = type_is_float(return_type) ? XMM0 : RAX;
		Result result_rax = result_make_reg(return_type, reg);

		Result result = result_make_reg(return_type, type_is_float(return_type) ? register_alloc_float(emit) : register_alloc(emit));
		emit_mov(emit, &result, &result_rax);

		return result;
	}

	return result_make_u64(return_type, 0);
}

static Result codegen_expression(Code_Emitter * emit, AST_Expression const * expr) {
	assert(expr->height >= 0);

	emit_trace_push_expr(emit, expr);
	
	emit->line = expr->line;
	error_set_line(expr->line);

	Result result;
	switch (expr->type) {
		case AST_EXPRESSION_CONST: result = codegen_expression_const(emit, expr); break;
		case AST_EXPRESSION_VAR:   result = codegen_expression_var  (emit, expr); break;

		case AST_EXPRESSION_ARRAY_ACCESS:  result = codegen_expression_array_access (emit, expr); break;
		case AST_EXPRESSION_STRUCT_MEMBER: result = codegen_expression_struct_member(emit, expr); break;
		
		case AST_EXPRESSION_CAST:   result = codegen_expression_cast  (emit, expr); break;
		case AST_EXPRESSION_SIZEOF: result = codegen_expression_sizeof(emit, expr); break;

		case AST_EXPRESSION_OPERATOR_BIN:  result = codegen_expression_op_bin (emit, expr); break;
		case AST_EXPRESSION_OPERATOR_PRE:  result = codegen_expression_op_pre (emit, expr); break;
		case AST_EXPRESSION_OPERATOR_POST: result = codegen_expression_op_post(emit, expr); break;

		case AST_EXPRESSION_CALL_FUNC: result = codegen_expression_call_func(emit, expr); break;

		default: error_internal();
	}
	
	emit_trace_pop(emit);

	return result;
}

static void codegen_statement_statements(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENTS);

	if (stat->stat_stats.head) codegen_statement(emit, stat->stat_stats.head);
	if (stat->stat_stats.cons) codegen_statement(emit, stat->stat_stats.cons);
}

static void codegen_statement_expression(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXPR);

	if (emit->emit_debug_lines) {
		char str[1024];
		ast_print_expression(stat->stat_expr.expr, str, sizeof(str));
		emit_asm(emit, "; %s\n", str);
	}

	Result result = codegen_expression(emit, stat->stat_expr.expr);
	result_free(emit, &result);
}

static void codegen_statement_def_var(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DEF_VAR);
	
	if (emit->emit_debug_lines) {
		char str[1024];
		ast_print_statement(stat, str, sizeof(str));
		emit_asm(emit, "; %s", str);
	}

	char const * var_name = stat->stat_def_var.name;
	
	Variable const * var = scope_get_variable(emit->current_scope, var_name);

	if (scope_is_global(emit->current_scope)) {
		if (stat->stat_def_var.assign) {
			AST_Expression const * literal_expr = stat->stat_def_var.assign->expr_op_bin.expr_right;
			
			if (literal_expr->type != AST_EXPRESSION_CONST) {
				error(ERROR_CODEGEN, "Cannot initialize global '%s' to non-constant", var_name);
			}

			Token const * literal = &literal_expr->expr_const.token;

			switch (literal->type) {
				case TOKEN_LITERAL_INT: 
				case TOKEN_LITERAL_BOOL: emit_global(emit, var, literal->sign, literal->value_int); break;
				case TOKEN_LITERAL_STRING: emit_string_literal(emit, var_name, literal->value_str); break;
				case TOKEN_LITERAL_F32: emit_f32_literal(emit, var_name, literal->value_float);  break;
				case TOKEN_LITERAL_F64: emit_f64_literal(emit, var_name, literal->value_double); break;

				default: error_internal();
			}
		} else {
			emit_global(emit, var, false, 0);
		}
	} else {
		if (stat->stat_def_var.assign) {
			Result result = codegen_expression(emit, stat->stat_def_var.assign);
			result_free(emit, &result);
		} else {
			Result result_var_address = variable_get_address(var);
			
			int type_size = type_get_size(var->type, emit->current_scope);

			if (type_is_struct(var->type) || type_is_array(var->type)) {
				char var_address[RESULT_STR_BUF_SIZE]; result_to_str(var_address, sizeof(var_address), &result_var_address);

				emit_asm(emit, "lea rdi, %s ; zero initialize '%s'\n", var_address, var_name);
				emit_asm(emit, "xor rax, rax\n");
				emit_asm(emit, "mov ecx, %i\n", type_size);
				emit_asm(emit, "rep stosb\n");
			} else {
				char var_address[RESULT_STR_BUF_SIZE]; result_to_str_sized(var_address, sizeof(var_address), &result_var_address, type_size);

				emit_asm(emit, "mov %s, 0 ; zero initialize '%s'\n", var_address, var_name);
			}
		}
	}
}

static void codegen_statement_def_func(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DEF_FUNC);

	char const * function_name = stat->stat_def_func.function_def->name;
	emit->current_function_name = function_name;

	emit_label(emit, function_name);
	emit->indent++;

	emit_asm(emit, "push rbp ; save RBP\n");
	emit_asm(emit, "mov rbp, rsp ; stack frame\n");
	
	Scope * scope_args = stat->stat_def_func.scope_args;

	int arg_count = stat->stat_def_func.function_def->arg_count;

	// Push first 4 arguments (if we have that many) that were passed in registers onto the stack
	for (int i = 0; i < MIN(arg_count, 4); i++) {
		Variable * var = scope_get_variable(scope_args, stat->stat_def_func.function_def->args[i].name);

		char const * mov = "mov";
		if (type_is_f32(var->type)) {
			mov = "movss";
		} else if (type_is_f64(var->type)) {
			mov = "movsd";
		}

		int type_size = type_get_size(var->type, scope_args);
		Register reg = get_call_register(i, type_is_float(var->type));

		emit_asm(emit, "%s %s [rbp + %i], %s ; push arg %i\n", mov, get_word_name(type_size), var->offset, get_reg_name(reg, type_size), i);
	}

	// Reserve space on stack for local variables
	Variable_Buffer * stack_frame = stat->stat_def_func.buffer_vars;
	
	int stack_frame_size_aligned = (stack_frame->size + 15) & ~15; // Round up to next 16 byte border

	for (int i = 0; i < stack_frame->vars_len; i++) {
		stack_frame->vars[i].offset -= stack_frame_size_aligned;
	}

	if (stack_frame->vars_len > 0) {
		emit_asm(emit, "mov rax, %i\n", stack_frame_size_aligned);
		emit_asm(emit, "call __chkstk\n");
		emit_asm(emit, "sub rsp, rax ; reserve stack space for %i locals\n", stack_frame->vars_len);
	}

	emit_asm(emit, "\n");

	// Function body
	codegen_statement(emit, stat->stat_def_func.body);

	emit_asm(emit, "xor rax, rax ; Default return value 0\n");
	emit_asm(emit, "L_function_%s_exit:\n", stat->stat_def_func.function_def->name);
	emit_asm(emit, "mov rsp, rbp\n");
	emit_asm(emit, "pop rbp\n");
	emit_asm(emit, "ret\n");
	emit_asm(emit, "\n");
	
	emit->current_scope = emit->current_scope->prev;
	emit->current_function_name = NULL;

	emit->indent--;
}

static void codegen_statement_extern(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXTERN);

	emit_asm(emit, "extern %s\n", stat->stat_extern.function_def->name);
}

static void codegen_statement_export(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXPORT);

	char const * name = stat->stat_export.name;

	// Lookup function to make sure it exists
	Function_Def * function_def = scope_get_function_def(emit->current_scope, name);
	if (function_def == NULL) {
		error(ERROR_SCOPE, "Trying to export function '%s' that is not in scope!", name);
	}

	emit_asm(emit, "global %s\n", name);
	emit_asm(emit, "export %s\n", name);
}

static void codegen_statement_if(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_IF);

	if (emit->emit_debug_lines) {
		char str[1024];
		ast_print_expression(stat->stat_if.condition, str, sizeof(str));
		emit_asm(emit, "; if (%s)\n", str);
	}
	
	int  label = get_new_label(emit);
	char label_if_true[LABEL_STR_BUF_SIZE]; sprintf_s(label_if_true, sizeof(label_if_true), "L_if_true_%i", label);
	char label_if_else[LABEL_STR_BUF_SIZE];	sprintf_s(label_if_else, sizeof(label_if_else), "L_if_else_%i", label);
	char label_if_exit[LABEL_STR_BUF_SIZE];	sprintf_s(label_if_exit, sizeof(label_if_exit), "L_if_exit_%i", label);

	assert(!flag_is_set(emit->flags, EMIT_FLAG_INSIDE_CONDITION));

	flag_set(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);
	emit->current_condition_label_true  = label_if_true;
	emit->current_condition_label_false = stat->stat_if.case_false ? label_if_else : label_if_exit;
	Result result = codegen_expression(emit, stat->stat_if.condition);
	emit->current_condition_label_true  = NULL;
	emit->current_condition_label_false = NULL;
	flag_unset(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);
	
	if (result.form == RESULT_IMMEDIATE) {
		if (result.u64 != 0) {
			codegen_statement(emit, stat->stat_if.case_true);
		} else if (stat->stat_if.case_false != NULL) {
			codegen_statement(emit, stat->stat_if.case_false);
		}
		return;
	}
	
	Condition_Code condition_code = condition_code_invert(result_get_condition_code(emit, &result));
	result_free(emit, &result);

	if (stat->stat_if.case_false == NULL) {
		emit_jcc  (emit, condition_code, label_if_exit);
		emit_label(emit, label_if_true);

		emit->indent++;
		codegen_statement(emit, stat->stat_if.case_true);
		emit->indent--;
	} else {
		emit_jcc  (emit, condition_code, label_if_else);
		emit_label(emit, label_if_true);

		emit->indent++;
		codegen_statement(emit, stat->stat_if.case_true);
		emit->indent--;

		emit_jmp  (emit, label_if_exit);
		emit_label(emit, label_if_else);

		emit->indent++;
		codegen_statement(emit, stat->stat_if.case_false);
		emit->indent--;
	}

	emit_label(emit, label_if_exit);
}

static void codegen_statement_while(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_WHILE);
	
	if (emit->emit_debug_lines) {
		char str[1024];
		ast_print_expression(stat->stat_while.condition, str, sizeof(str));
		emit_asm(emit, "; while (%s)\n", str);
	}

	int  label = get_new_label(emit);
	char label_while_loop[LABEL_STR_BUF_SIZE]; sprintf_s(label_while_loop, sizeof(label_while_loop), "L_while_loop_%i", label);
	char label_while_true[LABEL_STR_BUF_SIZE]; sprintf_s(label_while_true, sizeof(label_while_true), "L_while_true_%i", label);
	char label_while_exit[LABEL_STR_BUF_SIZE]; sprintf_s(label_while_exit, sizeof(label_while_exit), "L_while_exit_%i", label);

	emit_label(emit, label_while_loop);

	assert(!flag_is_set(emit->flags, EMIT_FLAG_INSIDE_CONDITION));

	flag_set(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);
	emit->current_condition_label_true  = label_while_true;
	emit->current_condition_label_false = label_while_exit;
	Result result = codegen_expression(emit, stat->stat_while.condition);
	emit->current_condition_label_true  = NULL;
	emit->current_condition_label_false = NULL;
	flag_unset(&emit->flags, EMIT_FLAG_INSIDE_CONDITION);
	
	if (result.form == RESULT_IMMEDIATE) {
		if (result.u64 == 0) {
			return;
		}
	} else {
		Condition_Code cc = condition_code_invert(result_get_condition_code(emit, &result));
		emit_jcc(emit, cc, label_while_exit);
		result_free(emit, &result);
	}
	
	emit_label(emit, label_while_true);

	int prev_loop_label = emit->current_loop_label;

	emit->current_loop_label = label;
	emit->indent++;
	codegen_statement(emit, stat->stat_while.body);
	emit->indent--;
	emit->current_loop_label = prev_loop_label;

	emit_jmp  (emit, label_while_loop);
	emit_label(emit, label_while_exit);
}

static void codegen_statement_break(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_BREAK);

	if (emit->current_loop_label == -1) {
		error(ERROR_CODEGEN, "Cannot use 'break' outside of loop!");
	}

	if (emit->emit_debug_lines) {
		emit_asm(emit, "; break\n");
	}

	char label_while_exit[LABEL_STR_BUF_SIZE]; sprintf_s(label_while_exit, sizeof(label_while_exit), "L_while_exit_%i", emit->current_loop_label);
	emit_jmp(emit, label_while_exit);
}

static void codegen_statement_continue(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_CONTINUE);
	
	if (emit->current_loop_label == -1) {
		error(ERROR_CODEGEN, "Cannot use 'continue' outside of loop!");
	}
	
	if (emit->emit_debug_lines) {
		emit_asm(emit, "; continue\n");
	}

	char label_while_loop[LABEL_STR_BUF_SIZE]; sprintf_s(label_while_loop, sizeof(label_while_loop), "L_while_loop_%i", emit->current_loop_label);
	emit_jmp(emit, label_while_loop);
}

static void codegen_statement_return(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_RETURN);
	
	Function_Def * func_def = scope_get_function_def(emit->current_scope, emit->current_function_name);
	assert(func_def);

	if (stat->stat_return.expr) {
		if (emit->emit_debug_lines) {
			char str[1024];
			ast_print_expression(stat->stat_return.expr, str, sizeof(str));
			emit_asm(emit, "; return %s\n", str);
		}

		Result result = codegen_expression(emit, stat->stat_return.expr);

		if (!types_unifiable(result.type, func_def->return_type)) {
			char str_ret_type[128]; type_to_string(result.type,           str_ret_type, sizeof(str_ret_type));
			char str_def_type[128]; type_to_string(func_def->return_type, str_def_type, sizeof(str_def_type));

			type_error(emit, "Return statement in function '%s' cannot unify types '%s' and '%s'", emit->current_function_name, str_ret_type, str_def_type);
		}
		
		if (type_is_void(result.type)) {
			type_error(emit, "Return statement in function '%s' cannot return void expression!", emit->current_function_name);
		}

		Result result_ret;
		if (type_is_float(result.type)) {
			result_ret = result_make_reg(result.type, XMM0);
		} else {
			result_ret = result_make_reg(result.type, RAX);
		}

		if (result.by_address) {
			emit_lea(emit, &result_ret, &result);
		} else {
			emit_mov(emit, &result_ret, &result);
		}

		result_free(emit, &result);
	} else {	
		if (emit->emit_debug_lines) {
			emit_asm(emit, "; return\n");
		}

		if (!type_is_void(func_def->return_type)) {
			char str_ret_type[128];
			type_to_string(func_def->return_type, str_ret_type, sizeof(str_ret_type));

			type_error(emit, "Return statement without expression in function '%s', which should return type '%s'", emit->current_function_name, str_ret_type);
		}

		emit_asm(emit, "xor rax, rax\n");
	}
	
	emit_asm(emit, "jmp L_function_%s_exit\n", emit->current_scope->variable_buffer->name);
}

static void codegen_statement_block(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_BLOCK);

	emit->current_scope = stat->stat_block.scope;

	if (stat->stat_block.stat) codegen_statement(emit, stat->stat_block.stat);

	emit->current_scope = emit->current_scope->prev;
}

static void codegen_statement_program(Code_Emitter * emit, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_PROGRAM);

	emit->current_scope = stat->stat_program.global_scope;

	Function_Def * main_def = scope_lookup_function_def(emit->current_scope, "main");
	
	emit_asm(emit, "__chkstk: ; touch stack at page size offsets (4096 bytes) to ensure pages are commited\n");
	emit->indent++;
	emit_asm(emit, "xor r11, r11\n");
	emit_asm(emit, "lea r10, [rsp + 8]\n");
	emit_asm(emit, "sub r10, rax\n");
	emit_asm(emit, "cmovb r10, r11\n");
	emit_asm(emit, "mov r11, QWORD gs:[16]\n");
	emit_asm(emit, "cmp r10, r11\n");
	emit_asm(emit, "jae __chkstk_done\n");
	emit_asm(emit, "and r10w, 0F000h\n\n");
	emit_asm(emit, "__chkstk_commit_page:\n");
	emit_asm(emit, "lea r11, [r11 - 4096]\n");
	emit_asm(emit, "mov BYTE [r11], 0\n");
	emit_asm(emit, "cmp r10, r11\n");
	emit_asm(emit, "jne __chkstk_commit_page\n\n");
	emit_asm(emit, "__chkstk_done:\n");
	emit_asm(emit, "ret\n\n");
	emit->indent--;

	if (main_def) {
		if (emit->needs_main) {
			bool main_valid = false;
			bool needs_args = false;

			if (main_def->arg_count == 0) {
				main_valid = true;
			} else if (main_def->arg_count == 1) {
				main_valid = type_is_string(main_def->args[0].type); // Argument to main should be string
				needs_args = true;
			}

			if (!main_valid) {
				type_error(emit, "Entry point 'main' has invalid arguments!\nShould have either 0 arguments or 1 (char *)\n");
			}

			emit_asm(emit, "global __start\n");
			emit_asm(emit, "__start:\n");
			emit->indent++;
			emit_asm(emit, "sub rsp, 32 + 8\n");

			if (needs_args) {
				emit_asm(emit, "call GetCommandLineA\n");
				emit_asm(emit, "mov [rsp], rax\n");
				emit_asm(emit, "mov rcx, rax\n");
			}

			emit_asm(emit, "call main\n");
			emit_asm(emit, "add rsp, 32\n");

			emit_asm(emit, "mov ecx, eax\n");
			emit_asm(emit, "call ExitProcess\n\n");
			emit->indent--;
		} else {
			puts("WARNING: A 'main' function was defined but was not needed for entry!");
		}
	} else if (emit->needs_main) {
		error(ERROR_CODEGEN, "No function 'main' was defined!");
	}

	codegen_statement(emit, stat->stat_program.stat);
}

static void codegen_statement(Code_Emitter * emit, AST_Statement const * stat) {
	if (stat->type != AST_STATEMENTS) emit_trace_push_stat(emit, stat);

	emit->line = stat->line;

	switch (stat->type) {
		case AST_STATEMENT_PROGRAM: codegen_statement_program(emit, stat); break;

		case AST_STATEMENTS:      codegen_statement_statements(emit, stat); break;
		case AST_STATEMENT_BLOCK: codegen_statement_block     (emit, stat); break;

		case AST_STATEMENT_EXPR: codegen_statement_expression(emit, stat); break;

		case AST_STATEMENT_DEF_VAR:  codegen_statement_def_var (emit, stat); break;
		case AST_STATEMENT_DEF_FUNC: codegen_statement_def_func(emit, stat); break;
		case AST_STATEMENT_EXTERN:   codegen_statement_extern  (emit, stat); break;
		case AST_STATEMENT_EXPORT:   codegen_statement_export  (emit, stat); break;

		case AST_STATEMENT_IF:    codegen_statement_if   (emit, stat); break;
		case AST_STATEMENT_WHILE: codegen_statement_while(emit, stat); break;

		case AST_STATEMENT_BREAK:    codegen_statement_break   (emit, stat); break;
		case AST_STATEMENT_CONTINUE: codegen_statement_continue(emit, stat); break;
		case AST_STATEMENT_RETURN:   codegen_statement_return  (emit, stat); break;

		default: error_internal();
	}

	// No regs should be active after a statement is complete
	assert(emit->reg_mask == 0); 
	
	if (stat->type != AST_STATEMENTS && stat->type != AST_STATEMENT_BLOCK) emit_asm(emit, "\n");
	if (stat->type != AST_STATEMENTS) emit_trace_pop(emit);
}

char const * codegen_program(AST_Statement const * program, bool needs_main) {
	Code_Emitter emit = make_emit(needs_main, true);
	
	static char const * header =
		"; Generated by Lang compiler\n\n"
		
		"extern GetCommandLineA\n"
		"extern ExitProcess\n\n"
		
		"section .text\n";

	emit_asm(&emit, header);
	codegen_statement(&emit, program);

	emit_asm(&emit, "section .data\n");
	emit_asm(&emit, "%s dq 0x5f000000 ; 2^63 as float\n",          const_f32_to_u64_name);
	emit_asm(&emit, "%s dq 0x43e0000000000000 ; 2^63 as double\n", const_f64_to_u64_name);

	for (int i = 0; i < emit.data_seg_len; i++) {
		emit_asm(&emit, "%s\n", emit.data_seg_vals[i]);
	}
	
	if (emit.bss_len > 0) {
		emit_asm(&emit, "\nsection .bss\n");

		for (int i = 0; i < emit.bss_len; i++) {
			emit_asm(&emit, "%s\n", emit.bss[i]);
		}
	}

	return emit.code;
}

#include "Codegen.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#define SCRATCH_REG_COUNT       7
#define SCRATCH_REG_COUNT_FLOAT 8

typedef enum Context_Flags {
	CTX_FLAG_VAR_BY_ADDRESS = 1
} Context_Flags;

typedef struct Context {
	int reg_mask_scratch; // Scratch registers currently in use
	int reg_mask_call;    // Call    registers currently in use

	int indent;

	int label;
	int current_loop_label;

	unsigned flags;

	Scope * current_scope;

	char * code;
	int    code_len;
	int    code_cap;

	char const ** data_seg_vals;
	int           data_seg_len;
	int           data_seg_cap;
} Context;

static void context_init(Context * ctx) {
	ctx->reg_mask_scratch = 0;
	ctx->reg_mask_call    = 0;

	ctx->indent = 0;

	ctx->label = 0;
	ctx->current_loop_label = -1;

	ctx->flags = 0;

	ctx->current_scope = NULL;

	ctx->code_len = 0;
	ctx->code_cap = 512;
	ctx->code     = malloc(ctx->code_cap);

	ctx->data_seg_len  = 0;
	ctx->data_seg_cap  = 16;
	ctx->data_seg_vals = malloc(ctx->data_seg_cap * sizeof(char const *));
}

static int context_reg_request(Context * ctx) {
	for (int i = 0; i < SCRATCH_REG_COUNT; i++) {
		if ((ctx->reg_mask_scratch & (1 << i)) == 0) {
			ctx->reg_mask_scratch |= (1 << i);

			return i;
		}
	}

	printf("ERROR: Out of registers!");
	abort();
}

static int context_reg_request_float(Context * ctx) {
	for (int i = SCRATCH_REG_COUNT; i < SCRATCH_REG_COUNT + SCRATCH_REG_COUNT_FLOAT; i++) {
		if ((ctx->reg_mask_scratch & (1 << i)) == 0) {
			ctx->reg_mask_scratch |= (1 << i);

			return i;
		}
	}

	printf("ERROR: Out of XMM registers!");
	abort();
}

static void context_reg_free(Context * ctx, int reg) {
	if (reg == -1) return;

	assert(ctx->reg_mask_scratch & (1 << reg));

	ctx->reg_mask_scratch &= ~(1 << reg);
}

static void context_call_reg_reserve(Context * ctx, int reg) {
	assert(reg >= 0 && reg < 4);

	ctx->reg_mask_call |= (1 << reg);
}

static bool context_call_reg_is_reserved(Context * ctx, int reg) {
	assert(reg >= 0 && reg < 4);

	return ctx->reg_mask_call & (1 << reg);
}

static void context_call_reg_free(Context * ctx, int reg) {
	assert(reg >= 0 && reg < 4);

	ctx->reg_mask_call &= ~(1 << reg);
}

static int context_new_label(Context * ctx) {
	return ctx->label++;
}

static void context_flag_set(Context * ctx, Context_Flags flag) {
	ctx->flags |= flag;
}

static void context_flag_unset(Context * ctx, Context_Flags flag) {
	ctx->flags &= ~flag;
}

static void variable_get_address(Variable const * var, char * address, int address_size) {
	if (var->is_global) {
		sprintf_s(address, address_size, "REL %s", var->name); // Globals by name
	} else {
		sprintf_s(address, address_size, "rbp + %i", var->offset); // Locals / Arguments relative to stack frame pointer
	}
}

static void type_error(char const * msg, ...) {
	va_list args;
	va_start(args, msg);

	char str_error[512];
	vsprintf_s(str_error, sizeof(str_error), msg, args);

	printf("TYPE ERROR: %s\n", str_error);

	va_end(args);
	
	abort();
}

static void context_emit_code(Context * ctx, char const * fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char new_code[1024];

	// Add indentation based on current Context
	char const * indent = "    ";
	int          indent_len = strlen(indent);

	for (int i = 0; i < ctx->indent; i++) {
		memcpy(new_code + i * indent_len, indent, indent_len);
	}
	
	// Format code
	vsprintf_s(new_code + ctx->indent * indent_len, sizeof(new_code) - ctx->indent * indent_len, fmt, args);
	
	va_end(args);
	
	// Resize code buffer if necessary
	int new_code_len = strlen(new_code);

	int new_length = ctx->code_len + new_code_len;
	if (new_length >= ctx->code_cap) {
		ctx->code_cap *= 2;
		ctx->code = realloc(ctx->code, ctx->code_cap);
	}

	// Append formatted code
	memcpy(ctx->code + ctx->code_len, new_code, new_code_len + 1);
	ctx->code_len = new_length;
}

static void context_add_data_seg(Context * ctx, char const * data) {
	if (ctx->data_seg_len == ctx->data_seg_cap) {
		ctx->data_seg_cap *= 2;
		ctx->data_seg_vals = realloc(ctx->data_seg_vals, ctx->data_seg_cap * sizeof(char const *));
	}

	ctx->data_seg_vals[ctx->data_seg_len++] = data;
}

// Adds global variable to data segment
static void context_add_global(Context * ctx, Variable * var, int value) {
	int var_name_len = strlen(var->name);

	int    global_size = var_name_len + 5 + 32;
	char * global = malloc(global_size);

	if (type_is_struct(var->type)) {
		if (value != 0) {
			type_error("Cannot initialize global struct variable '%s' with value '%i'", var->name, value);
		}

		// Fill struct with 0 quad words
		sprintf_s(global, global_size, "%s dq 0", var->name);

		int struct_size = type_get_size(var->type, ctx->current_scope) / 8;

		for (int i = 1; i < struct_size; i++) {
			strcat_s(global, global_size, ", 0");
		}
	} else {
		sprintf_s(global, global_size, "%s dq %i", var->name, value);
	}

	context_add_data_seg(ctx, global);
}

// Adds float literal to data segment
static void context_add_float_literal(Context * ctx, char const * flt_name, char const * flt_lit) {
	int flt_name_len = strlen(flt_name);
	int flt_lit_len  = strlen(flt_lit);

	int    str_lit_len = flt_name_len + 4 + flt_lit_len + 1;
	char * str_lit = malloc(str_lit_len);
	sprintf_s(str_lit, str_lit_len, "%s dq %s", flt_name, flt_lit);

	context_add_data_seg(ctx, str_lit);
}

// Adds string literal to data segment
static void context_add_string_literal(Context * ctx, char const * str_name, char const * str_lit) {
	int str_name_len = strlen(str_name);
	int str_lit_len  = strlen(str_lit);

	int    str_lit_cpy_size = str_name_len + 6 + str_lit_len * 9 + 1;
	char * str_lit_cpy      = malloc(str_lit_cpy_size);

	int idx = sprintf_s(str_lit_cpy, str_lit_cpy_size, "%s db ", str_name);

	str_lit_cpy[idx++] = '\"';

	char const * curr = str_lit;
	while (*curr) {
		if (*curr == '\\') {
			switch (*(curr + 1)) {
				case 'r':  strcpy_s(str_lit_cpy + idx, str_lit_cpy_size - idx, "\", 0Dh, \""); idx += 9; break;
				case 'n':  strcpy_s(str_lit_cpy + idx, str_lit_cpy_size - idx, "\", 0Ah, \""); idx += 9; break;
				case 't':  strcpy_s(str_lit_cpy + idx, str_lit_cpy_size - idx, "\", 09h, \""); idx += 9; break;

				case '\\': str_lit_cpy[idx++] = '\\'; break;

				default: abort();
			}

			curr += 2;
		} else {	
			str_lit_cpy[idx++] = *curr;

			curr++;
		}
	}
	
	str_lit_cpy[idx++] = '\"';
	str_lit_cpy[idx++] = '\0';

	strcat_s(str_lit_cpy, str_lit_cpy_size, ", 0");

	context_add_data_seg(ctx, str_lit_cpy);
}

// Result of an AST_Expression
typedef struct Result {
	int    reg;
	Type const * type;
} Result;

static Result codegen_expression(Context * ctx, AST_Expression const * expr);
static void   codegen_statement (Context * ctx, AST_Statement  const * stat);

static char const * get_reg_name_scratch(int reg_index, int size) {
	static char const * reg_names_8bit [SCRATCH_REG_COUNT] = {  "bl", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b" };
	static char const * reg_names_16bit[SCRATCH_REG_COUNT] = {  "bx", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" };
	static char const * reg_names_32bit[SCRATCH_REG_COUNT] = { "ebx", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" };
	static char const * reg_names_64bit[SCRATCH_REG_COUNT] = { "rbx", "r10",  "r11",  "r12",  "r13",  "r14",  "r15"  };

	static char const * reg_names_float_128bit[SCRATCH_REG_COUNT_FLOAT] = { "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11" };

	if (reg_index < SCRATCH_REG_COUNT) {
		switch (size) {
			case 1: return reg_names_8bit [reg_index];
			case 2: return reg_names_16bit[reg_index];
			case 4: return reg_names_32bit[reg_index];
			case 8: return reg_names_64bit[reg_index];

			default: abort();
		}
	} else {
		return reg_names_float_128bit[reg_index - SCRATCH_REG_COUNT];
	}
}

static char const * get_reg_name_call(int reg_index, int type_size, bool is_float) {
	static char const * reg_names_8bit [4] = {  "cl",  "dl", "r8b", "r9b" };
	static char const * reg_names_16bit[4] = {  "cx",  "dx", "r8w", "r9w" };
	static char const * reg_names_32bit[4] = { "ecx", "edx", "r8d", "r9d" };
	static char const * reg_names_64bit[4] = { "rcx", "rdx", "r8",  "r9"  }; 

	static char const * reg_names_float[4] = { "xmm0", "xmm1", "xmm2", "xmm3" };

	if (is_float) {
		return reg_names_float[reg_index];
	} else {
		switch (type_size) {
			case 1: return reg_names_8bit [reg_index];
			case 2: return reg_names_16bit[reg_index];
			case 4: return reg_names_32bit[reg_index];
			case 8: return reg_names_64bit[reg_index];

			default: abort();
		}
	}
}

static char const * get_word_name(int size) {
	switch (size) {
		case 1:  return "BYTE";
		case 2:  return "WORD";
		case 4:  return "DWORD";
		default: return "QWORD";
	}
}

static Result codegen_expression_const(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_CONST);

	Result result;
	result.reg = context_reg_request(ctx);
	
	Token_Type literal_type = expr->expr_const.token.type;

	switch (literal_type) {
		case TOKEN_LITERAL_STRING: {
			result.type = make_type_pointer(make_type_u8());

			char str_lit_name[128];
			sprintf_s(str_lit_name, sizeof(str_lit_name), "lit_str_%i", ctx->data_seg_len);

			context_add_string_literal(ctx, str_lit_name, expr->expr_const.token.value_str);
			context_emit_code(ctx, "lea %s, [REL %s]\n", get_reg_name_scratch(result.reg, 8), str_lit_name);

			break;
		}

		case TOKEN_LITERAL_INT: {
			unsigned long long value = expr->expr_const.token.value_int;

			// Find smallest possible type that can contain the value
			if (expr->expr_const.token.sign) {
				if (value >= CHAR_MIN && value <= CHAR_MAX) {
					result.type = make_type_i8();
				} else if (value >= SHRT_MIN && value <= SHRT_MAX) {
					result.type = make_type_i16();
				} else if (value >= INT_MIN && value <= INT_MAX) {
					result.type = make_type_i32();
				} else {
					result.type = make_type_i64();
				}

				context_emit_code(ctx, "mov %s, %lld\n", get_reg_name_scratch(result.reg, 8), value);
			} else {
				if (value <= UCHAR_MAX) {
					result.type = make_type_u8();
				} else if (value <= USHRT_MAX) {
					result.type = make_type_u16();
				} else if (value <= UINT_MAX) {
					result.type = make_type_u32();
				} else {
					result.type = make_type_u64();
				}
				
				context_emit_code(ctx, "mov %s, %llu\n", get_reg_name_scratch(result.reg, 8), value);
			}

			break;
		}

		case TOKEN_LITERAL_F32: {
			context_reg_free(ctx, result.reg);
			result.reg = context_reg_request_float(ctx);
			result.type = make_type_f32();
			
			char str_lit_name[128];
			sprintf_s(str_lit_name, sizeof(str_lit_name), "lit_flt_%i", ctx->data_seg_len);

			unsigned flt;
			memcpy(&flt, &expr->expr_const.token.value_float, 4);

			char str_lit_flt[8 + 4 + 32 + 1];
			sprintf_s(str_lit_flt, sizeof(str_lit_flt), "%xh ; %ff", flt, expr->expr_const.token.value_float);

			context_add_float_literal(ctx, str_lit_name, str_lit_flt);
			context_emit_code(ctx, "movss %s, DWORD [REL %s]\n", get_reg_name_scratch(result.reg, 8), str_lit_name);

			break;
		}

		case TOKEN_LITERAL_F64: {
			context_reg_free(ctx, result.reg);
			result.reg = context_reg_request_float(ctx);
			result.type = make_type_f64();
			
			char str_lit_name[128];
			sprintf_s(str_lit_name, sizeof(str_lit_name), "lit_flt_%i", ctx->data_seg_len);

			unsigned long long dbl;
			memcpy(&dbl, &expr->expr_const.token.value_double, 8);

			char str_lit_flt[16 + 4 + 32 + 1];
			sprintf_s(str_lit_flt, sizeof(str_lit_flt), "%llxh ; %f", dbl, expr->expr_const.token.value_double);

			context_add_float_literal(ctx, str_lit_name, str_lit_flt);
			context_emit_code(ctx, "movsd %s, QWORD [REL %s]\n", get_reg_name_scratch(result.reg, 8), str_lit_name);

			break;
		}

		case TOKEN_LITERAL_BOOL: {
			result.type = make_type_bool();

			context_emit_code(ctx, "mov %s, %i\n", get_reg_name_scratch(result.reg, 8), expr->expr_const.token.value_int);

			break;
		}

		default: abort();
	}

	return result;
}

static int codegen_deref_address(Context * ctx, int reg, Type const * type, char const * var_address) {
	int type_size = type_get_size(type, ctx->current_scope);

	int reg_out  = reg;
	int reg_size = 8;

	char const * mov = "mov";

	if (type_is_f32(type)) {
		reg_out = context_reg_request_float(ctx);

		mov = "movss";
	} else if (type_is_f64(type)) {
		reg_out = context_reg_request_float(ctx);

		mov = "movsd";
	} else if (type_size < 8) {
		if (type_is_integral_signed(type)) {
			mov = "movsx"; // Signed extend
		} else if (type_size < 4) {
			mov = "movzx"; // Zero extend
		} else {
			reg_size = 4; // 32bit movs will automatically zero extend the highest 32 bits
		}
	}

	context_emit_code(ctx, "%s %s, %s [%s]\n", mov, get_reg_name_scratch(reg_out, reg_size), get_word_name(type_size), var_address);
	
	if (reg_out != reg) context_reg_free(ctx, reg);

	return reg_out;
}

static Result codegen_expression_var(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_VAR);

	char     const * var_name = expr->expr_var.name;
	Variable const * var = scope_get_variable(ctx->current_scope, var_name);

	char var_address[32];
	variable_get_address(var, var_address, sizeof(var_address));

	bool by_address = ctx->flags & CTX_FLAG_VAR_BY_ADDRESS;

	Result result;
	if (type_is_float(var->type) && !by_address) {
		result.reg = context_reg_request_float(ctx);
	} else {
		result.reg = context_reg_request(ctx);
	}
	result.type = var->type;

	bool is_global_char_ptr =
		var->is_global &&
		type_is_pointer(var->type) &&
		type_is_u8(var->type->base);

	if (by_address || is_global_char_ptr || type_is_array(var->type)) {
		context_emit_code(ctx, "lea %s, QWORD [%s] ; get address of '%s'\n", get_reg_name_scratch(result.reg, 8), var_address, var_name);
	} else {
		result.reg = codegen_deref_address(ctx, result.reg, result.type, var_address, var_name);
	}

	return result;
}

static Result codegen_expression_struct_member(Context * ctx, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_STRUCT_MEMBER);

	bool var_by_address = ctx->flags & CTX_FLAG_VAR_BY_ADDRESS; 

	// Evaluate LHS by address
	context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);
	Result result = codegen_expression(ctx, expr->expr_struct_member.expr);

	if (!type_is_struct(result.type)) {
		char str_type[128];
		type_to_string(result.type, str_type, sizeof(str_type));

		type_error("Operator '.' requires left operand to be a struct type. Type was '%s'", str_type);
	}

	// Lookup the struct member by name
	Struct_Def * struct_def = scope_get_struct_def(ctx->current_scope, result.type->struct_name);
	Variable   * var_member = scope_get_variable(struct_def->member_scope, expr->expr_struct_member.member_name);

	context_emit_code(ctx, "add %s, %i ; member offset '%s'\n", get_reg_name_scratch(result.reg, 8), var_member->offset, var_member->name);
	
	result.type = var_member->type;

	// Check if we need to return by value
	if (!var_by_address) {
		context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);

		if (type_is_primitive(result.type)) {
			result.reg = codegen_deref_address(ctx, result.reg, result.type,  get_reg_name_scratch(result.reg, 8), NULL);
		}
	}

	return result;
}

static Result codegen_expression_cast(Context * ctx, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_CAST);

	Result result = codegen_expression(ctx, expr->expr_cast.expr);

	Type const * type_old = result.type;
	Type const * type_new = expr->expr_cast.new_type;

	if (type_is_integral(type_old)) {
		if (type_is_f32(type_new)) {
			int reg = context_reg_request_float(ctx);

			context_emit_code(ctx, "cvtsi2ss %s, %s\n", get_reg_name_scratch(reg, 8), get_reg_name_scratch(result.reg, 8));

			context_reg_free(ctx, result.reg);
			result.reg = reg;
		} else if (type_is_f64(type_new)) {
			int reg = context_reg_request_float(ctx);

			context_emit_code(ctx, "cvtsi2sd %s, %s\n", get_reg_name_scratch(reg, 8), get_reg_name_scratch(result.reg, 8));
		
			context_reg_free(ctx, result.reg);
			result.reg = reg;
		}
	} else if (type_is_f32(type_old)) {
		if (type_is_integral(type_new)) {
			int reg = context_reg_request(ctx);

			context_emit_code(ctx, "cvttss2si %s, %s\n", get_reg_name_scratch(reg, 8), get_reg_name_scratch(result.reg, 8));
		
			context_reg_free(ctx, result.reg);
			result.reg = reg;
		} else if (type_is_f64(type_new)) {
			context_emit_code(ctx, "cvtss2sd %s, %s\n", get_reg_name_scratch(result.reg, 8), get_reg_name_scratch(result.reg, 8));
		}
	} else if (type_is_f64(type_old)) {
		if (type_is_integral(type_new)) {
			int reg = context_reg_request(ctx);

			context_emit_code(ctx, "cvttsd2si %s, %s\n", get_reg_name_scratch(reg, 8), get_reg_name_scratch(result.reg, 8));
		
			context_reg_free(ctx, result.reg);
			result.reg = reg;
		} else if (type_is_f32(type_new)) {
			context_emit_code(ctx, "cvtsd2ss %s, %s\n", get_reg_name_scratch(result.reg, 8), get_reg_name_scratch(result.reg, 8));
		}
	}

	result.type = type_new;

	return result;
}

static Result codegen_expression_sizeof(Context * ctx, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_SIZEOF);

	Type const * type = expr->expr_sizeof.type;

	Result result;
	result.reg  = context_reg_request(ctx);
	result.type = make_type_u32();

	int size = type_get_size(type, ctx->current_scope);

	char str_type[128];
	type_to_string(type, str_type, sizeof(str_type));

	context_emit_code(ctx, "mov %s, %i ; sizeof '%s'\n", get_reg_name_scratch(result.reg, 8), size, str_type);

	return result;
}

// Helper function used by relational and equality operators
static void codegen_compare(Context * ctx, char const * cmp_inst, char const * set_inst, int reg_left, int reg_right, int reg_result) {
	char const * reg_name_left  = get_reg_name_scratch(reg_left,  8);
	char const * reg_name_right = get_reg_name_scratch(reg_right, 8);

	char const * reg_name_result_full = get_reg_name_scratch(reg_result, 8);
	char const * reg_name_result_byte = get_reg_name_scratch(reg_result, 1);

	context_emit_code(ctx, "%s %s, %s\n", cmp_inst, reg_name_left, reg_name_right);
	context_emit_code(ctx, "%s %s\n",     set_inst, reg_name_result_byte);
	context_emit_code(ctx, "and %s, 1\n",    reg_name_result_byte);
	context_emit_code(ctx, "movzx %s, %s\n", reg_name_result_full, reg_name_result_byte);
}

// Helper function used by relational operators
static void codegen_relational(Context * ctx, char const * operator, char const * set_inst, Result * result_left, Result * result_right) {
	if ((type_is_integral(result_left->type) && type_is_integral(result_right->type)) ||
		(type_is_pointer (result_left->type) && type_is_pointer (result_right->type) && types_unifiable(result_left->type, result_right->type))
	) {
		codegen_compare(ctx, "cmp", set_inst, result_left->reg, result_right->reg, result_left->reg);
	} else if (type_is_f32(result_left->type) && type_is_f32(result_right->type)) {
		context_reg_free(ctx, result_left->reg);
		int reg = context_reg_request(ctx);

		codegen_compare(ctx, "comiss", set_inst, result_left->reg, result_right->reg, reg);
		result_left->reg = reg;
	} else if (type_is_f64(result_left->type) && type_is_f64(result_right->type)) {
		context_reg_free(ctx, result_left->reg);
		int reg = context_reg_request(ctx);

		codegen_compare(ctx, "comisd", set_inst, result_left->reg, result_right->reg, reg);
		result_left->reg = reg;
	} else {
		type_error("Operator '%s' requires two integral, float, or pointer types", operator);
	}

	result_left->type = make_type_bool();
}

static Result codegen_expression_op_bin(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_BIN);

	Token_Type operator = expr->expr_op_bin.token.type;
		
	AST_Expression const * expr_left  = expr->expr_op_bin.expr_left;
	AST_Expression const * expr_right = expr->expr_op_bin.expr_right;
	
	Result result_left, result_right;

	// Assignment operator is handled separately, because it needs the lhs by address
	if (operator == TOKEN_ASSIGN) {
		if (expr_left->type != AST_EXPRESSION_VAR && 
			expr_left->type != AST_EXPRESSION_OPERATOR_PRE &&
			expr_left->type != AST_EXPRESSION_STRUCT_MEMBER
		) {
			type_error("Operator '=' requires left operand to be an lvalue");
		}

		// Traverse tallest subtree first
		if (expr->expr_op_bin.expr_left->height >= expr->expr_op_bin.expr_right->height) {
			// Evaluate lhs by address
			context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);
			result_left  = codegen_expression(ctx, expr_left);
			context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);

			result_right = codegen_expression(ctx, expr_right);
		} else {
			result_right = codegen_expression(ctx, expr_right);

			// Evaluate lhs by address
			context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);
			result_left = codegen_expression(ctx, expr_left);
			context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);
		}

		int type_size_left  = type_get_size(result_left .type, ctx->current_scope);
		int type_size_right = type_get_size(result_right.type, ctx->current_scope);

		if (!types_unifiable(result_left.type, result_right.type)) {
			char str_type_left [128];
			char str_type_right[128];

			type_to_string(result_left .type, str_type_left,  sizeof(str_type_left));
			type_to_string(result_right.type, str_type_right, sizeof(str_type_right));

			type_error("Cannot assign instance of type '%s' a value of type '%s'", str_type_left, str_type_right);
		} else if (
			type_is_primitive(result_left .type) &&
			type_is_primitive(result_right.type) &&
			type_size_right > type_size_left
		) {
			char str_type_left [128];
			char str_type_right[128];

			type_to_string(result_left .type, str_type_left,  sizeof(str_type_left));
			type_to_string(result_right.type, str_type_right, sizeof(str_type_right));

			type_error("Implicit narrowing conversion from type '%s' to '%s' is not allowed. Explicit cast required", str_type_right, str_type_left);
		}

		if (type_is_struct(result_left.type) && type_is_struct(result_right.type)) {
			int struct_size = type_get_size(result_left.type, ctx->current_scope);

			context_emit_code(ctx, "mov rdi, %s\n", get_reg_name_scratch(result_left .reg, 8));
			context_emit_code(ctx, "mov rsi, %s\n", get_reg_name_scratch(result_right.reg, 8));
			context_emit_code(ctx, "mov ecx, %i\n", struct_size);
			context_emit_code(ctx, "rep movsb\n");			
		} else {
			char const * mov = "mov";
			if (type_is_f32(result_right.type)) {
				mov = "movss";
			} else if (type_is_f64(result_right.type)) {
				mov = "movsd";
			}

			context_emit_code(ctx, "%s %s [%s], %s\n", mov, get_word_name(type_size_left), get_reg_name_scratch(result_left.reg, 8), get_reg_name_scratch(result_right.reg, type_size_left));
		}

		context_reg_free(ctx, result_right.reg);

		result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);

		return result_left;
	}
	
	// Handle operator that require short-circuit evaluation separately
	if (operator ==  TOKEN_OPERATOR_LOGICAL_AND) {
		int label = context_new_label(ctx);

		result_left  = codegen_expression(ctx, expr_left);
		char const * reg_name_left = get_reg_name_scratch(result_left.reg, 8);

		context_emit_code(ctx, "test %s, %s\n", reg_name_left, reg_name_left);
		context_emit_code(ctx, "je L_land_false_%i\n", label);
		
		result_right = codegen_expression(ctx, expr_right);
		char const * reg_name_right = get_reg_name_scratch(result_right.reg, 8);

		context_emit_code(ctx, "test %s, %s\n", reg_name_right, reg_name_right);
		context_emit_code(ctx, "je L_land_false_%i\n", label);
		context_emit_code(ctx, "mov %s, 1\n",   reg_name_left);
		context_emit_code(ctx, "jmp L_land_exit_%i\n", label);
		context_emit_code(ctx, "L_land_false_%i:\n", label);
		context_emit_code(ctx, "mov %s, 0\n",   reg_name_left);
		context_emit_code(ctx, "L_land_exit_%i:\n",  label);
	
		if (!type_is_bool(result_left.type) || !type_is_bool(result_right.type)) {
			type_error("Operator '&&' requires two boolean operands");
		}
		
		context_reg_free(ctx, result_right.reg);

		return result_left;
	} else if (operator == TOKEN_OPERATOR_LOGICAL_OR) {
		int label = context_new_label(ctx);
		
		result_left  = codegen_expression(ctx, expr_left);
		char const * reg_name_left = get_reg_name_scratch(result_left.reg, 8);

		context_emit_code(ctx, "test %s, %s\n", reg_name_left, reg_name_left);
		context_emit_code(ctx, "jne L_lor_true_%i\n", label);

		result_right = codegen_expression(ctx, expr_right);
		char const * reg_name_right = get_reg_name_scratch(result_right.reg, 8);

		context_emit_code(ctx, "test %s, %s\n", reg_name_right, reg_name_right);
		context_emit_code(ctx, "jne L_lor_true_%i\n", label);
		context_emit_code(ctx, "mov %s, 0\n",   reg_name_left);
		context_emit_code(ctx, "jmp L_lor_exit_%i\n", label);
		context_emit_code(ctx, "L_lor_true_%i:\n", label);
		context_emit_code(ctx, "mov %s, 1\n",   reg_name_left);
		context_emit_code(ctx, "L_lor_exit_%i:\n",  label);
			
		if (!type_is_bool(result_left.type) || !type_is_bool(result_right.type)) {
			type_error("Operator '||' requires two boolean operands");
		}
		
		context_reg_free(ctx, result_right.reg);

		return result_left;
	}

	// Traverse tallest subtree first
	if (expr->expr_op_bin.expr_left->height >= expr->expr_op_bin.expr_right->height) {
		result_left  = codegen_expression(ctx, expr_left);
		result_right = codegen_expression(ctx, expr_right);
	} else {
		result_right = codegen_expression(ctx, expr_right);
		result_left  = codegen_expression(ctx, expr_left);
	}

	char const * reg_name_left  = get_reg_name_scratch(result_left .reg, 8);
	char const * reg_name_right = get_reg_name_scratch(result_right.reg, 8);

	// Emit correct instructions based on operator type
	switch (operator) {
		case TOKEN_OPERATOR_MULTIPLY: {
			if (type_is_integral(result_left.type) && type_is_integral(result_right.type)) {
				context_emit_code(ctx, "imul %s, %s\n", reg_name_left, reg_name_right);

				result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);
			} else if (type_is_f32(result_left.type) && type_is_f32(result_right.type)) {
				context_emit_code(ctx, "mulss %s, %s\n", reg_name_left, reg_name_right);
			} else if (type_is_f64(result_left.type) && type_is_f64(result_right.type)) {
				context_emit_code(ctx, "mulsd %s, %s\n", reg_name_left, reg_name_right);
			} else {
				type_error("Operator '*' only works with integral types");
			}

			break;
		}

		case TOKEN_OPERATOR_DIVIDE: {
			if (type_is_integral(result_left.type) && type_is_integral(result_right.type)) {
				context_emit_code(ctx, "mov rax, %s\n", reg_name_left);
				context_emit_code(ctx, "cdq\n");
				context_emit_code(ctx, "idiv %s\n",     reg_name_right);
				context_emit_code(ctx, "mov %s, rax\n", reg_name_left);

				result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);
			} else if (type_is_f32(result_left.type) && type_is_f32(result_right.type)) {
				context_emit_code(ctx, "divss %s, %s\n", reg_name_left, reg_name_right);
			} else if (type_is_f64(result_left.type) && type_is_f64(result_right.type)) {
				context_emit_code(ctx, "divsd %s, %s\n", reg_name_left, reg_name_right);
			} else {
				type_error("Operator '/' only works with integral or float types");
			}

			break;
		}

		case TOKEN_OPERATOR_MODULO: {
			context_emit_code(ctx, "mov rax, %s\n", reg_name_left);
			context_emit_code(ctx, "cdq\n");
			context_emit_code(ctx, "idiv %s\n",     reg_name_right);
			context_emit_code(ctx, "mov %s, rdx\n", reg_name_left); // RDX contains remainder after division

			if (type_is_integral(result_left.type) && type_is_integral(result_right.type)) {
				result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);
			} else {
				type_error("Operator '%' only works with integral types");
			}

			break;
		}

		case TOKEN_OPERATOR_PLUS: {
			char const * add = "add";
			if (type_is_f32(result_left.type)) {
				add = "addss";
			} else if (type_is_f64(result_left.type)) {
				add = "addsd";
			}
			context_emit_code(ctx, "%s %s, %s\n", add, reg_name_left, reg_name_right);

			if (type_is_arithmetic(result_left.type) && type_is_arithmetic(result_right.type)) { // arithmetic + arithmetic --> arithmetic
				result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);
			} else if (type_is_pointer(result_left.type) && type_is_integral(result_right.type)) { // pointer + integral --> pointer
				// Resulting type is pointer, do nothing
			} else if (type_is_array(result_left.type) && type_is_integral(result_right.type)) { // array + integral --> pointer
				result_left.type = make_type_pointer(result_left.type->base);
			} else {
				type_error("Left of operator '+' must be integral, float, array, or pointer type, right must be integral or float type!");
			}

			break;
		}

		case TOKEN_OPERATOR_MINUS: {
			char const * sub = "sub";
			if (type_is_f32(result_left.type)) {
				sub = "subss";
			} else if (type_is_f64(result_left.type)) {
				sub = "subsd";
			}
			context_emit_code(ctx, "%s %s, %s\n", sub, reg_name_left, reg_name_right);

			if (type_is_arithmetic(result_left.type) && type_is_arithmetic(result_right.type)) { // arithmetic - arithmetic --> arithmetic
				result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);
			} else if (type_is_pointer(result_left.type) && type_is_integral(result_right.type)) { // pointer - integral --> pointer
				// Resulting type is pointer, do nothing
			} else if (type_is_pointer(result_left.type) && type_is_pointer(result_right.type) && types_unifiable(result_left.type, result_right.type)) { // pointer - pointer --> integral
				result_left.type = make_type_i64();
			} else {
				type_error("Operator '-' cannot cannot have integral type on the left and pointer type on the right");
			}

			break;
		}

		case TOKEN_OPERATOR_SHIFT_LEFT: {
			context_emit_code(ctx, "mov rcx, %s\n", reg_name_right);
			context_emit_code(ctx, "shl %s, cl\n",  reg_name_left);

			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error("Operator '<<' requires two operands of integral type");
			}

			result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);

			break;
		}

		case TOKEN_OPERATOR_SHIFT_RIGHT: {
			context_emit_code(ctx, "mov rcx, %s\n", reg_name_right);
			if (type_is_integral_signed(result_left.type)) {
				context_emit_code(ctx, "sar %s, cl\n", reg_name_left);
			} else if (type_is_integral_unsigned(result_left.type)) {
				context_emit_code(ctx, "shr %s, cl\n", reg_name_left);
			}

			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error("Operator '>>' requires two operands of integral type");
			}

			result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);

			break;
		}

		case TOKEN_OPERATOR_LT:    codegen_relational(ctx, "<",  "setl",  &result_left, &result_right); break;
		case TOKEN_OPERATOR_LT_EQ: codegen_relational(ctx, "<=", "setle", &result_left, &result_right); break;
		case TOKEN_OPERATOR_GT:    codegen_relational(ctx, ">",  "setg",  &result_left, &result_right); break;
		case TOKEN_OPERATOR_GT_EQ: codegen_relational(ctx, ">=", "setge", &result_left, &result_right); break;

		case TOKEN_OPERATOR_EQ: {
			codegen_compare(ctx, "cmp", "sete", result_left.reg, result_right.reg, result_left.reg);
			
			if (!types_unifiable(result_left.type, result_right.type)) {
				type_error("Operator '==' requires equal types on both sides");
			}
			
			result_left.type = make_type_bool();
			
			break;
		}

		case TOKEN_OPERATOR_NE: {
			codegen_compare(ctx, "cmp", "setne", result_left.reg, result_right.reg, result_left.reg);
			
			if (!types_unifiable(result_left.type, result_right.type)) {
				type_error("Operator '!=' requires equal types on both sides");
			}

			result_left.type = make_type_bool();
			
			break;
		}

		case TOKEN_OPERATOR_BITWISE_AND: {
			context_emit_code(ctx, "and %s, %s\n", reg_name_left, reg_name_right);

			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error("Operator '&' requires two operands of integral type");
			}

			result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);

			break;
		}

		case TOKEN_OPERATOR_BITWISE_XOR: {
			context_emit_code(ctx, "xor %s, %s\n", reg_name_left, reg_name_right);

			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error("Operator '^' requires two operands of integral type");
			}

			result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);

			break;
		}

		case TOKEN_OPERATOR_BITWISE_OR: {
			context_emit_code(ctx, "or %s, %s\n", reg_name_left, reg_name_right);

			if (!type_is_integral(result_left.type) || !type_is_integral(result_right.type)) {
				type_error("Operator '|' requires two operands of integral type");
			}

			result_left.type = types_unify(result_left.type, result_right.type, ctx->current_scope);

			break;
		}

		default: abort();
	}

	context_reg_free(ctx, result_right.reg);

	return result_left;
}

static Result codegen_expression_op_pre(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_PRE);

	AST_Expression * operand  = expr->expr_op_pre.expr;
	Token_Type       operator = expr->expr_op_pre.token.type;

	Result result;

	// Check if this is a pointer operator
	if (operator == TOKEN_OPERATOR_BITWISE_AND) {
		if (operand->type != AST_EXPRESSION_VAR && 
			operand->type != AST_EXPRESSION_STRUCT_MEMBER
		) {
			type_error("Operator '&' can only take address of a variable or struct member");
		}

		bool by_address = ctx->flags & CTX_FLAG_VAR_BY_ADDRESS;
		context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);

		result = codegen_expression(ctx, operand);
		result.type = make_type_pointer(result.type);

		if (!by_address) context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);

		return result;
	} else if (operator == TOKEN_OPERATOR_MULTIPLY) {
		bool by_address = ctx->flags & CTX_FLAG_VAR_BY_ADDRESS;

		context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS); // Unset temporarily

		result = codegen_expression(ctx, operand);
		
		if (!type_is_pointer(result.type)) {
			char str_type[128];
			type_to_string(result.type, str_type, sizeof(str_type));

			type_error("Cannot dereference non-pointer type '%s'", str_type);
		} else if (type_is_void_pointer(result.type)) {
			type_error("Cannot dereference 'void *'");
		}

		result.type = result.type->base; // Remove 1 level of indirection

		if (by_address) {
			context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS); // Reset if this flag was previously set
		} else {
			result.reg = codegen_deref_address(ctx, result.reg, result.type, get_reg_name_scratch(result.reg, 8));
		}

		return result;
	}

	bool by_address = false;

	if (operator == TOKEN_OPERATOR_INC || operator == TOKEN_OPERATOR_DEC) {
		by_address = ctx->flags & CTX_FLAG_VAR_BY_ADDRESS;
		context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);
	}

	result = codegen_expression(ctx, operand);

	if (!by_address) context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);

	char const * reg_name = get_reg_name_scratch(result.reg, 8);

	switch (operator) {
		case TOKEN_OPERATOR_INC: {
			int type_size = type_get_size(result.type, ctx->current_scope);

			int reg_address = context_reg_request(ctx);

			context_emit_code(ctx, "mov %s, %s\n", get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(result.reg, 8));
			
			result.reg = codegen_deref_address(ctx, result.reg, result.type, get_reg_name_scratch(result.reg, 8));

			context_emit_code(ctx, "inc %s\n", get_reg_name_scratch(result.reg, 8));
			context_emit_code(ctx, "mov %s [%s], %s\n", get_word_name(type_size), get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(result.reg, type_size));

			context_reg_free(ctx, reg_address);

			if (!type_is_integral(result.type)) {
				type_error("Operator '++' requires operand of integral type");
			}

			break;
		}

		case TOKEN_OPERATOR_DEC: {
			int type_size = type_get_size(result.type, ctx->current_scope);

			int reg_address = context_reg_request(ctx);

			context_emit_code(ctx, "mov %s, %s\n", get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(result.reg, 8));
			
			result.reg = codegen_deref_address(ctx, result.reg, result.type, get_reg_name_scratch(result.reg, 8));

			context_emit_code(ctx, "dec %s\n", get_reg_name_scratch(result.reg, 8));
			context_emit_code(ctx, "mov %s [%s], %s\n", get_word_name(type_size), get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(result.reg, type_size));

			context_reg_free(ctx, reg_address);

			if (!type_is_integral(result.type)) {
				type_error("Operator '--' requires operand of integral type");
			}

			break;
		}

		case TOKEN_OPERATOR_PLUS: {
			// Unary plus is a no-op, no code is emitted

			if (!type_is_integral(result.type)) {
				type_error("Operator '+' requires operand of integral type");
			}

			break;
		}
		case TOKEN_OPERATOR_MINUS: {
			context_emit_code(ctx, "neg %s\n", reg_name);
			
			if (!type_is_integral(result.type)) {
				type_error("Operator '-' requires operand of integral type");
			}
			
			break;
		}

		case TOKEN_OPERATOR_BITWISE_NOT: {
			context_emit_code(ctx, "not %s\n", reg_name);

			if (!type_is_integral(result.type)) {
				type_error("Operator '~' requires operand of integral type");
			}

			break;
		}

		case TOKEN_OPERATOR_LOGICAL_NOT: {
			int label = context_new_label(ctx);

			context_emit_code(ctx, "test %s, %s\n", reg_name, reg_name);
			context_emit_code(ctx, "jne L_lnot_false_%i\n", label);
			context_emit_code(ctx, "mov %s, 1\n",   reg_name);
			context_emit_code(ctx, "jmp L_lnot_exit_%i\n", label);
			context_emit_code(ctx, "L_lnot_false_%i:\n", label);
			context_emit_code(ctx, "mov %s, 0\n",   reg_name);
			context_emit_code(ctx, "L_lnot_exit_%i:\n",  label);

			if (!type_is_bool(result.type)) {
				type_error("Operator '!' requires operand of boolean type");
			}

			break;
		}

		default: abort();
	}

	return result;
}

static Result codegen_expression_op_post(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_POST);

	AST_Expression * operand  = expr->expr_op_pre.expr;
	Token_Type       operator = expr->expr_op_pre.token.type;

	bool by_address = ctx->flags & CTX_FLAG_VAR_BY_ADDRESS;

	context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);
	Result result = codegen_expression(ctx, operand);

	if (!by_address) context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);

	char const * reg_name = get_reg_name_scratch(result.reg, 8);

	switch (operator) {
		case TOKEN_OPERATOR_INC: {
			int type_size = type_get_size(result.type, ctx->current_scope);

			int reg_address = context_reg_request(ctx);
			int reg_value   = context_reg_request(ctx);

			context_emit_code(ctx, "mov %s, %s\n", get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(result.reg, 8));
			
			result.reg = codegen_deref_address(ctx, result.reg, result.type, get_reg_name_scratch(result.reg, 8));

			context_emit_code(ctx, "mov %s, %s\n", get_reg_name_scratch(reg_value, 8), get_reg_name_scratch(result.reg, 8));
			context_emit_code(ctx, "inc %s\n",     get_reg_name_scratch(reg_value, 8));
			context_emit_code(ctx, "mov %s [%s], %s\n", get_word_name(type_size), get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(reg_value, type_size));

			context_reg_free(ctx, reg_address);
			context_reg_free(ctx, reg_value);

			if (!type_is_integral(result.type)) {
				type_error("Operator '++' requires operand of integral type");
			}

			break;
		}

		case TOKEN_OPERATOR_DEC: {
			int type_size = type_get_size(result.type, ctx->current_scope);

			int reg_address = context_reg_request(ctx);
			int reg_value   = context_reg_request(ctx);

			context_emit_code(ctx, "mov %s, %s\n", get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(result.reg, 8));
			
			result.reg = codegen_deref_address(ctx, result.reg, result.type, get_reg_name_scratch(result.reg, 8));

			context_emit_code(ctx, "mov %s, %s\n", get_reg_name_scratch(reg_value, 8), get_reg_name_scratch(result.reg, 8));
			context_emit_code(ctx, "dec %s\n",     get_reg_name_scratch(reg_value, 8));
			context_emit_code(ctx, "mov %s [%s], %s\n", get_word_name(type_size), get_reg_name_scratch(reg_address, 8), get_reg_name_scratch(reg_value, type_size));

			context_reg_free(ctx, reg_address);
			context_reg_free(ctx, reg_value);

			if (!type_is_integral(result.type)) {
				type_error("Operator '--' requires operand of integral type");
			}

			break;
		}

		default: abort();
	}

	return result;
}

static Result codegen_expression_call_func(Context * ctx, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_CALL_FUNC);
	
	// Count call arguments
	int call_arg_count = expr->expr_call.arg_count;

	// Get function definition
	Function_Def * def_func = scope_get_function_def(ctx->current_scope, expr->expr_call.function_name);

	if (call_arg_count != def_func->arg_count) {
		type_error("Incorrect number of arguments to function '%s'! Provided %i, needed %i",
			expr->expr_call.function_name, call_arg_count, def_func->arg_count
		);
	}

	bool pushed_regs_call   [4]                 = { false, false, false, false };
	bool pushed_regs_scratch[SCRATCH_REG_COUNT] = { false, false, false, false, false, false, false };

	for (int i = 0; i < SCRATCH_REG_COUNT; i++) {
		if (ctx->reg_mask_scratch & (1 << i)) {
			pushed_regs_scratch[i] = true;

			context_emit_code(ctx, "push %s ; preserve\n", get_reg_name_scratch(i, 8));
		}
	}

	// Get total size of arguments
	int arg_size = 0;

	for (int i = 0; i < expr->expr_call.arg_count; i++) {
		AST_Def_Arg * def_arg = &def_func->args[i];

		if (i < 4) {
			if (context_call_reg_is_reserved(ctx, i)) {
				// If the call registers is already in use (i.e. this is a nested
				// function call), push the register on the stack to preserve it
				pushed_regs_call[i] = true;

				context_emit_code(ctx, "push %s ; preserve\n", get_reg_name_call(i, 8, type_is_float(def_arg->type)));
			} else {
				context_call_reg_reserve(ctx, i);
			}

			arg_size += 8;
		} else {
			int type_size  = type_get_size (def_arg->type, ctx->current_scope);
			int type_align = type_get_align(def_arg->type, ctx->current_scope);

			align(&arg_size, type_align);
			arg_size += type_size;
		}
	}

	if (arg_size < 32) {
		arg_size = 32; // Needs at least 32 bytes for shadow space
	} else {
		arg_size = (arg_size + 15) & ~15; // Round up to next multiple of 16 bytes to ensure stack alignment
	}

	// Reserve stack space for arguments
	context_emit_code(ctx, "sub rsp, %i ; reserve shadow space and %i arguments\n", arg_size, call_arg_count);

	// Evaluate arguments and put them into the right register / stack address
	// The first 4 arguments go in registers, the rest spills onto the stack
	int arg_offset = 0;

	for (int i = 0; i < expr->expr_call.arg_count; i++) {
		Result result_arg = codegen_expression(ctx, expr->expr_call.args[i].expr);

		AST_Def_Arg * def_arg = &def_func->args[i];

		if (!types_unifiable(result_arg.type, def_arg->type)) {
			char str_type_given   [128];
			char str_type_expected[128];

			type_to_string(result_arg.type, str_type_given,    sizeof(str_type_given));
			type_to_string(def_arg->type,   str_type_expected, sizeof(str_type_expected));

			type_error("Argument %i in function call to '%s' has incorrect type. Given type: '%s', expected type: '%s'",
				i + 1, expr->expr_call.function_name, str_type_given, str_type_expected
			);
		}

		char const * mov = "mov";
		if (type_is_f32(result_arg.type)) {
			mov = "movss";
		} else if (type_is_f64(result_arg.type)) {
			mov = "movsd";
		}

		if (i < 4) {
			context_emit_code(ctx, "%s %s, %s ; arg %i\n", mov, get_reg_name_call(i, 8, type_is_float(result_arg.type)), get_reg_name_scratch(result_arg.reg, 8), i + 1);

			arg_offset += 8;
		} else {
			int type_size  = type_get_size (def_arg->type, ctx->current_scope);
			int type_align = type_get_align(def_arg->type, ctx->current_scope);
			
			char const * word = get_word_name(type_size);

			context_emit_code(ctx, "%s %s [rsp + %i], %s ; arg %i\n", mov, word, arg_offset, get_reg_name_scratch(result_arg.reg, type_size), i + 1);

			align(&arg_offset, type_align);
			arg_offset += type_size;
		}

		context_reg_free(ctx, result_arg.reg);
	}

	context_emit_code(ctx, "call %s\n", expr->expr_call.function_name);

	context_emit_code(ctx, "add rsp, %i ; pop arguments\n", arg_size);

	// Check if any registers were pushed before this call and need to be restored
	for (int i = min(3, expr->expr_call.arg_count - 1); i >= 0; i--) {
		if (pushed_regs_call[i]) {
			context_emit_code(ctx, "pop %s ; restore\n", get_reg_name_call(i, 8, type_is_float(def_func->args[i].type)));
		} else {
			context_call_reg_free(ctx, i);
		}
	}

	for (int i = SCRATCH_REG_COUNT - 1; i >= 0; i--) {
		if (pushed_regs_scratch[i]) {
			context_emit_code(ctx, "pop %s ; restore\n", get_reg_name_scratch(i, 8));
		}
	}

	Result result;
	result.type = def_func->return_type;

	if (type_is_float(result.type)) {
		result.reg = context_reg_request_float(ctx);

		if (type_is_f32(result.type)) {
			context_emit_code(ctx, "movss %s, xmm0 ; get return value\n", get_reg_name_scratch(result.reg, 8));
		} else {
			context_emit_code(ctx, "movsd %s, xmm0 ; get return value\n", get_reg_name_scratch(result.reg, 8));
		}
	} else {
		result.reg = context_reg_request(ctx);

		context_emit_code(ctx, "mov %s, rax ; get return value\n", get_reg_name_scratch(result.reg, 8));
	}

	return result;
}

static Result codegen_expression(Context * ctx, AST_Expression const * expr) {
	assert(expr->height >= 0);

	switch (expr->type) {
		case AST_EXPRESSION_CONST:         return codegen_expression_const        (ctx, expr);
		case AST_EXPRESSION_VAR:           return codegen_expression_var          (ctx, expr);
		case AST_EXPRESSION_STRUCT_MEMBER: return codegen_expression_struct_member(ctx, expr);
		
		case AST_EXPRESSION_CAST:   return codegen_expression_cast  (ctx, expr);
		case AST_EXPRESSION_SIZEOF: return codegen_expression_sizeof(ctx, expr);

		case AST_EXPRESSION_OPERATOR_BIN:  return codegen_expression_op_bin (ctx, expr);
		case AST_EXPRESSION_OPERATOR_PRE:  return codegen_expression_op_pre (ctx, expr);
		case AST_EXPRESSION_OPERATOR_POST: return codegen_expression_op_post(ctx, expr);

		case AST_EXPRESSION_CALL_FUNC: return codegen_expression_call_func(ctx, expr);

		default: abort();
	}
}

static void codegen_statement_statements(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENTS);

	if (stat->stat_stats.head) codegen_statement(ctx, stat->stat_stats.head);
	if (stat->stat_stats.cons) codegen_statement(ctx, stat->stat_stats.cons);
}

static void codegen_statement_expression(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXPR);

	Result result = codegen_expression(ctx, stat->stat_expr.expr);
	context_reg_free(ctx, result.reg);
}

static void codegen_statement_def_var(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DEF_VAR);

	char const * var_name = stat->stat_def_var.name;
	
	Variable const * var = scope_get_variable(ctx->current_scope, var_name);

	if (scope_is_global(ctx->current_scope)) {
		if (stat->stat_def_var.assign) {
			AST_Expression const * literal      = stat->stat_def_var.assign->expr_op_bin.expr_right;
			Token_Type             literal_type = literal->expr_const.token.type;
			
			if (literal->type != AST_EXPRESSION_CONST) {
				printf("ERROR: Globals can only be initialized to constant values! Variable name: '%s'", var_name); 
				abort();
			}

			switch (literal_type) {
				case TOKEN_LITERAL_INT: 
				case TOKEN_LITERAL_BOOL: {
					context_add_global(ctx, var, literal->expr_const.token.value_int);

					break;
				}

				case TOKEN_LITERAL_STRING: {
					context_add_string_literal(ctx, var_name, literal->expr_const.token.value_str);

					break;
				}

				default: abort();
			}
		} else {
			context_add_global(ctx, var, 0);
		}
	} else {
		char var_address[32];
		variable_get_address(var, var_address, sizeof(var_address));

		int type_size = type_get_size(var->type, ctx->current_scope);

		if (stat->stat_def_var.assign) {
			Result result = codegen_expression(ctx, stat->stat_def_var.assign);

			context_reg_free(ctx, result.reg);
		} else {
			if (type_is_struct(var->type)) {
				int struct_size = type_get_size(var->type, ctx->current_scope);

				context_emit_code(ctx, "lea rdi, QWORD [%s] ; zero initialize '%s'\n", var_address, var_name);
				context_emit_code(ctx, "xor rax, rax\n");
				context_emit_code(ctx, "mov ecx, %i\n", struct_size);
				context_emit_code(ctx, "rep stosb\n");
			} else {
				context_emit_code(ctx, "mov %s [%s], 0 ; zero initialize '%s'\n", get_word_name(type_size), var_address, var_name);
			}
		}
	}
}

static void codegen_statement_def_func(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DEF_FUNC);

	context_emit_code(ctx, "%s:\n", stat->stat_def_func.function_def->name);
	ctx->indent++;

	context_emit_code(ctx, "push rbp ; save RBP\n");
	context_emit_code(ctx, "mov rbp, rsp ; stack frame\n");

	Scope * scope_args = stat->stat_def_func.scope_args;
	
	// Push arguments on stack
	int arg_count = stat->stat_def_func.function_def->arg_count;

	for (int i = 0; i < min(arg_count, 4); i++) {
		Variable * var = scope_get_variable(scope_args, stat->stat_def_func.function_def->args[i].name);

		char const * mov = "mov";
		if (type_is_f32(var->type)) {
			mov = "movss";
		} else if (type_is_f64(var->type)) {
			mov = "movsd";
		}

		int type_size = type_get_size(var->type, scope_args);
		context_emit_code(ctx, "%s %s [rbp + %i], %s ; push arg %i \n", mov, get_word_name(type_size), var->offset, get_reg_name_call(i, type_size, type_is_float(var->type)), i);
	}
	
	// Reserve space on stack for local variables
	Variable_Buffer * stack_frame = stat->stat_def_func.buffer_vars;
	
	int stack_frame_size_aligned = (stack_frame->size + 15) & ~15; // Round up to next 16 byte border

	for (int i = 0; i < stack_frame->vars_len; i++) {
		stack_frame->vars[i].offset -= stack_frame_size_aligned;
	}

	if (stack_frame->vars_len > 0) {  
		context_emit_code(ctx, "sub rsp, %i ; reserve stack space for %i locals\n", stack_frame_size_aligned, stack_frame->vars_len);
	}

	// Function body
	codegen_statement(ctx, stat->stat_def_func.body);

	context_emit_code(ctx, "xor rax, rax ; Default return value 0\n");
	context_emit_code(ctx, "L_function_%s_exit:\n", stat->stat_def_func.function_def->name);
	context_emit_code(ctx, "mov rsp, rbp\n");
	context_emit_code(ctx, "pop rbp\n");
	context_emit_code(ctx, "ret\n");
	context_emit_code(ctx, "\n");
	
	ctx->current_scope = ctx->current_scope->prev;

	ctx->indent--;
}

static void codegen_statement_extern(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXTERN);

	context_emit_code(ctx, "EXTERN %s\n\n", stat->stat_extern.function_def->name);
}

static void codegen_statement_if(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_IF);

	Result result = codegen_expression(ctx, stat->stat_if.condition);
	context_reg_free(ctx, result.reg);

	int label = context_new_label(ctx);
	context_emit_code(ctx, "cmp %s, 0\n", get_reg_name_scratch(result.reg, 8));

	if (stat->stat_if.case_false == NULL) {
		context_emit_code(ctx, "je L_exit%i\n", label);
		
		ctx->indent++;
		codegen_statement(ctx, stat->stat_if.case_true);
		ctx->indent--;
	} else {
		context_emit_code(ctx, "je L_else%i\n", label);

		ctx->indent++;
		codegen_statement(ctx, stat->stat_if.case_true);
		ctx->indent--;

		context_emit_code(ctx, "jmp L_exit%i\n", label);
		context_emit_code(ctx, "L_else%i:\n",    label);
		
		ctx->indent++;
		codegen_statement(ctx, stat->stat_if.case_false);
		ctx->indent--;
	}

	context_emit_code(ctx, "L_exit%i:\n", label);
}

static void codegen_statement_while(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_WHILE);

	int label = context_new_label(ctx);
	context_emit_code(ctx, "L_loop%i:\n", label);
	
	Result result = codegen_expression(ctx, stat->stat_while.condition);
	context_reg_free(ctx, result.reg);

	context_emit_code(ctx, "cmp %s, 0\n", get_reg_name_scratch(result.reg, 8));
	context_emit_code(ctx, "je L_exit%i\n", label);

	int prev_loop_label = ctx->current_loop_label;

	ctx->current_loop_label = label;
	ctx->indent++;
	codegen_statement(ctx, stat->stat_while.body);
	ctx->indent--;
	ctx->current_loop_label = prev_loop_label;

	context_emit_code(ctx, "jmp L_loop%i\n", label);
	context_emit_code(ctx, "L_exit%i:\n", label);
}

static void codegen_statement_break(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_BREAK);

	if (ctx->current_loop_label == -1) {
		printf("ERROR: Cannot use 'break' outside of loop!");
		abort();
	}

	context_emit_code(ctx, "jmp L_exit%i\n", ctx->current_loop_label);
}

static void codegen_statement_continue(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_CONTINUE);
	
	if (ctx->current_loop_label == -1) {
		printf("ERROR: Cannot use 'continue' outside of loop!");
		abort();
	}
	
	context_emit_code(ctx, "jmp L_loop%i\n", ctx->current_loop_label);
}

static void codegen_statement_return(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_RETURN);

	if (stat->stat_return.expr) {
		Result result = codegen_expression(ctx, stat->stat_return.expr);

		if (type_is_struct(result.type)) {
			type_error("Cannot return structs by value from function");
		}

		if (type_is_f32(result.type)) {
			context_emit_code(ctx, "movss xmm0, %s ; return via xmm0\n", get_reg_name_scratch(result.reg, 8));
		} else if (type_is_f64(result.type)) {
			context_emit_code(ctx, "movsd xmm0, %s ; return via xmm0\n", get_reg_name_scratch(result.reg, 8));
		} else {
			context_emit_code(ctx, "mov rax, %s ; return via rax\n", get_reg_name_scratch(result.reg, 8));
		}

		context_reg_free(ctx, result.reg);
	} else {
		context_emit_code(ctx, "mov rax, 0\n");
	}
	
	context_emit_code(ctx, "jmp L_function_%s_exit\n", ctx->current_scope->variable_buffer->name);
}

static void codegen_statement_block(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_BLOCK);

	ctx->current_scope = stat->stat_block.scope;

	if (stat->stat_block.stat) codegen_statement(ctx, stat->stat_block.stat);

	ctx->current_scope = ctx->current_scope->prev;
}

static void codegen_statement_program(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_PROGRAM);

	ctx->current_scope = stat->stat_program.global_scope;

	codegen_statement(ctx, stat->stat_program.stat);
}

static void codegen_statement(Context * ctx, AST_Statement const * stat) {
	switch (stat->type) {
		case AST_STATEMENT_PROGRAM: codegen_statement_program(ctx, stat); break;

		case AST_STATEMENTS:      codegen_statement_statements(ctx, stat); break;
		case AST_STATEMENT_BLOCK: codegen_statement_block     (ctx, stat); break;

		case AST_STATEMENT_EXPR: codegen_statement_expression(ctx, stat); break;

		case AST_STATEMENT_DEF_VAR:  codegen_statement_def_var (ctx, stat); break;
		case AST_STATEMENT_DEF_FUNC: codegen_statement_def_func(ctx, stat); break;
		case AST_STATEMENT_EXTERN:   codegen_statement_extern  (ctx, stat); break;

		case AST_STATEMENT_IF:    codegen_statement_if   (ctx, stat); break;
		case AST_STATEMENT_WHILE: codegen_statement_while(ctx, stat); break;

		case AST_STATEMENT_BREAK:    codegen_statement_break   (ctx, stat); break;
		case AST_STATEMENT_CONTINUE: codegen_statement_continue(ctx, stat); break;
		case AST_STATEMENT_RETURN:   codegen_statement_return  (ctx, stat); break;

		default: abort();
	}

	// No regs should be active after a statement is complete
	assert(ctx->reg_mask_scratch == 0); 
	assert(ctx->reg_mask_call    == 0);

	if (stat->type != AST_STATEMENTS && stat->type != AST_STATEMENT_BLOCK) context_emit_code(ctx, "\n");
}

char const * codegen_program(AST_Statement const * program) {
	Context ctx;
	context_init(&ctx);

	char const asm_init[] =
		"; Generated by Lang compiler\n\n"
		"GLOBAL main\n\n"
		"SECTION .code\n";

	context_emit_code(&ctx, asm_init);
	codegen_statement(&ctx, program);

	context_emit_code(&ctx, "SECTION .data\n");
	for (int i = 0; i < ctx.data_seg_len; i++) {
		context_emit_code(&ctx, "%s\n", ctx.data_seg_vals[i]);
	}

	return ctx.code;
}

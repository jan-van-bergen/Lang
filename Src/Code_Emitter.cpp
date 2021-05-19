#include "Code_Emitter.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "Error.h"

static Register const scratch_registers[] = { RBX, R10, R11, R12, R13, R14, R15 };

bool register_is_float(Register reg) {
	return reg >= XMM0 && reg <= XMM15;
}

bool register_is_scratch(Register reg) {
	if (reg >= XMM4 && reg <= XMM15) return true;

	for (int i = 0; i < ARRAY_COUNT(scratch_registers); i++) {
		if (reg == scratch_registers[i]) {
			return true;
		}
	}
	return false;
}

Register register_alloc(Code_Emitter * emit) {
	for (int i = 0; i < ARRAY_COUNT(scratch_registers); i++) {
		Register reg = scratch_registers[i];
		if (!flag_is_set(emit->reg_mask, 1 << reg)) {
			flag_set(&emit->reg_mask, 1 << reg);

			return reg;
		}
	}

	printf("ERROR: Out of registers!");
	error(ERROR_CODEGEN);
}

Register register_alloc_float(Code_Emitter * emit) {
	for (Register reg = XMM4; reg <= XMM15; reg++) {
		if (!flag_is_set(emit->reg_mask, 1 << reg)) {
			flag_set(&emit->reg_mask, 1 << reg);

			return reg;
		}
	}

	printf("ERROR: Out of XMM registers!");
	error(ERROR_CODEGEN);
}

bool register_is_reserved(Code_Emitter * emit, Register reg) {
	return flag_is_set(emit->reg_mask, 1 << reg);
}

void register_free(Code_Emitter * emit, Register reg) {
	if (reg == -1) return;

	assert(register_is_reserved(emit, reg));
	flag_unset(&emit->reg_mask, 1 << reg);
}

Register get_call_register(int argument_index, bool is_float) {
	if (argument_index >= 4) error(ERROR_INTERNAL);

	if (is_float) {
		return XMM0 + argument_index;
	} else {
		static Register call_registers[4] = { RCX, RDX, R8, R9 };
		return call_registers[argument_index];
	}
}

char const * get_reg_name(Register reg, int size) {
	static char const * reg_names_float[] = { "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };

	if (reg >= XMM0 && reg <= XMM15) {
		return reg_names_float[reg - XMM0];
	}

	static char const * reg_names_8bit[]  = { [RAX] = "al",  [RBX] = "bl",  [RCX] = "cl",  [RDX] = "dl",  [RSP] = "spl", [RBP] = "bpl", [RSI] = "sil", [RDI] = "dil", [R8]  = "r8b", [R9]  = "r9b", [R10] = "r10b", [R11] = "r11b", [R12] = "r12b", [R13] = "r13b", [R14] = "r14b", [R15] = "r15b" };
	static char const * reg_names_16bit[] = { [RAX] = "ax",  [RBX] = "bx",  [RCX] = "cx",  [RDX] = "dx",  [RSP] = "sp",  [RBP] = "bp",  [RSI] = "si",  [RDI] = "di",  [R8]  = "r8w", [R9]  = "r9w", [R10] = "r10w", [R11] = "r11w", [R12] = "r12w", [R13] = "r13w", [R14] = "r14w", [R15] = "r15w" };
	static char const * reg_names_32bit[] = { [RAX] = "eax", [RBX] = "ebx", [RCX] = "ecx", [RDX] = "edx", [RSP] = "esp", [RBP] = "ebp", [RSI] = "esi", [RDI] = "edi", [R8]  = "r8d", [R9]  = "r9d", [R10] = "r10d", [R11] = "r11d", [R12] = "r12d", [R13] = "r13d", [R14] = "r14d", [R15] = "r15d" };
	static char const * reg_names_64bit[] = { [RAX] = "rax", [RBX] = "rbx", [RCX] = "rcx", [RDX] = "rdx", [RSP] = "rsp", [RBP] = "rbp", [RSI] = "rsi", [RDI] = "rdi", [R8]  = "r8",  [R9]  = "r9",  [R10] = "r10",  [R11] = "r11",  [R12] = "r12",  [R13] = "r13",  [R14] = "r14",  [R15] = "r15"  };

	if (reg < RAX || reg > R15) error(ERROR_CODEGEN);

	switch (size) {
		case 1: return reg_names_8bit [reg];
		case 2: return reg_names_16bit[reg];
		case 4: return reg_names_32bit[reg];
		case 8: return reg_names_64bit[reg];

		default: error(ERROR_CODEGEN);
	}
}

char const * get_word_name(int size) {
	switch (size) {
		case 1:  return "BYTE";
		case 2:  return "WORD";
		case 4:  return "DWORD";
		default: return "QWORD";
	}
}

Code_Emitter make_emit(bool needs_main, bool emit_debug_lines) {
	int const CODE_CAP = 512;
	int const DATA_CAP = 16;
	int const BSS_CAP  = 16;

	return (Code_Emitter) {
		.needs_main       = needs_main,
		.emit_debug_lines = emit_debug_lines,

		.reg_mask = 0,

		.indent = 0,

		.label = 0,
		.current_loop_label = -1,

		.flags = 0,

		.current_function_name         = NULL,
		.current_condition_label_true  = NULL,
		.current_condition_label_false = NULL,
		.current_scope                 = NULL,

		.code     = mem_alloc(CODE_CAP),
		.code_len = 0,
		.code_cap = CODE_CAP,

		.data_seg_vals = mem_alloc(DATA_CAP * sizeof(char const *)),
		.data_seg_len  = 0,
		.data_seg_cap  = DATA_CAP,
		
		.bss     = mem_alloc(DATA_CAP * sizeof(char const *)),
		.bss_len = 0,
		.bss_cap = DATA_CAP,

		.trace_stack_size = 0
	};
}

int get_new_label(Code_Emitter * emit) {
	return emit->label++;
}

void emit_asm(Code_Emitter * emit, char const * fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char new_code[1024] = { 0 };

	// Add indentation based on current Context
	char const * indent = "    ";
	int          indent_len = strlen(indent);

	for (int i = 0; i < emit->indent; i++) {
		memcpy(new_code + i * indent_len, indent, indent_len);
	}
	
	// Format code
	vsprintf_s(new_code + emit->indent * indent_len, sizeof(new_code) - emit->indent * indent_len, fmt, args);
	
	va_end(args);
	
	// Resize code buffer if necessary
	int new_code_len = (int)strlen(new_code);

	int new_length = emit->code_len + new_code_len;
	if (new_length >= emit->code_cap) {
		emit->code_cap *= 2;
		emit->code = mem_realloc(emit->code, emit->code_cap);
	}

	// Append formatted code
	memcpy(emit->code + emit->code_len, new_code, new_code_len + 1);
	emit->code_len = new_length;
}

void emit_data(Code_Emitter * emit, char const * data) {
	if (emit->data_seg_len == emit->data_seg_cap) {
		emit->data_seg_cap *= 2;
		emit->data_seg_vals = mem_realloc(emit->data_seg_vals, emit->data_seg_cap * sizeof(char const *));
	}

	emit->data_seg_vals[emit->data_seg_len++] = data;
}

void emit_bss(Code_Emitter * emit, char const * bss) {
	if (emit->bss_len == emit->bss_cap) {
		emit->bss_cap *= 2;
		emit->bss = mem_realloc(emit->bss, emit->bss_cap * sizeof(char const *));
	}

	emit->bss[emit->bss_len++] = bss;
}

void emit_trace_push_expr(Code_Emitter * emit, AST_Expression * expr) {
	if (emit->trace_stack_size == MAX_TRACE) error(ERROR_CODEGEN);

	Trace_Element * trace_elem = emit->trace_stack + emit->trace_stack_size++;
	trace_elem->type = TRACE_EXPRESSION;
	trace_elem->expr = expr;
}

void emit_trace_push_stat(Code_Emitter * emit, AST_Statement * stat) {
	if (emit->trace_stack_size == MAX_TRACE) error(ERROR_CODEGEN);

	Trace_Element * trace_elem = emit->trace_stack + emit->trace_stack_size++;
	trace_elem->type = TRACE_STATEMENT;
	trace_elem->stat = stat;
}

void emit_trace_pop(Code_Emitter * emit) {
	assert(emit->trace_stack_size > 0);
	emit->trace_stack_size--;
}

void emit_print_stack_trace(Code_Emitter * emit) {
	for (int i = 0; i < emit->trace_stack_size; i++) {
		char str[8 * 1024];

		Trace_Element * trace_elem = emit->trace_stack + i;
		if (trace_elem->type == TRACE_EXPRESSION) {
			puts("In expression:");
			ast_print_expression(trace_elem->expr, str, sizeof(str));
		} else {
			puts("In statement:");
			ast_print_statement(trace_elem->stat, str, sizeof(str));
		}

		puts(str);
		puts("");
	}
}

NO_RETURN void type_error(Code_Emitter * emit, char const * msg, ...) {
	emit_print_stack_trace(emit);

	va_list args;
	va_start(args, msg);

	char str_error[512];
	vsprintf_s(str_error, sizeof(str_error), msg, args);

	printf("TYPE ERROR: %s\n", str_error);

	va_end(args);
	
	error(ERROR_TYPECHECK);
}

// Adds global variable to data segment
void emit_global(Code_Emitter * emit, Variable * var, bool sign, uint64_t value) {
	int type_size = type_get_size(var->type, emit->current_scope);
	
	int    global_size = (int)strlen(var->name) + 64;
	char * global = mem_alloc(global_size);

	if (type_is_struct(var->type) || type_is_array(var->type) || value == 0) {
		if (value != 0) {
			type_error(emit, "Cannot initialize global aggregate '%s' with value '%llu'", var->name, value);
		}
		
		sprintf_s(global, global_size, "%s resb %u", var->name, type_size);

		emit_bss(emit, global);
	} else {
		char const * define_keyword = NULL;
		switch (type_size) {
			case 1: define_keyword = "db"; break; // define byte
			case 2: define_keyword = "dw"; break; // define word
			case 4: define_keyword = "dd"; break; // define dword
			case 8: define_keyword = "dq"; break; // define qword
			default: error(ERROR_INTERNAL);
		}

		if (sign) {
			sprintf_s(global, global_size, "%s %s %lld", var->name, define_keyword, value);
		} else {
			sprintf_s(global, global_size, "%s %s %llu", var->name, define_keyword, value);
		}
		
		emit_data(emit, global);
	}
}

// Adds float literal to data segment
void emit_f32_literal(Code_Emitter * emit, char const * flt_name, float value) {
	int flt_name_len = (int)strlen(flt_name);
	
	uint32_t hex_value; memcpy(&hex_value, &value, sizeof(float));

	int    str_lit_len = flt_name_len + 64;
	char * str_lit = mem_alloc(str_lit_len);
	sprintf_s(str_lit, str_lit_len, "%s dd 0x%x ; %ff", flt_name, hex_value, value);

	emit_data(emit, str_lit);
}

// Adds double literal to data segment
void emit_f64_literal(Code_Emitter * emit, char const * flt_name, double value) {
	int flt_name_len = (int)strlen(flt_name);
	
	uint64_t hex_value; memcpy(&hex_value, &value, sizeof(double));

	int    str_lit_len = flt_name_len + 64;
	char * str_lit = mem_alloc(str_lit_len);
	sprintf_s(str_lit, str_lit_len, "%s dq 0x%llx ; %ff", flt_name, hex_value, value);

	emit_data(emit, str_lit);
}

// Adds string literal to data segment
void emit_string_literal(Code_Emitter * emit, char const * str_name, char const * str_lit) {
	int str_name_len = (int)strlen(str_name);
	int str_lit_len  = (int)strlen(str_lit);

	int    str_lit_cpy_size = str_name_len + 6 + str_lit_len * 9 + 5;
	char * str_lit_cpy      = mem_alloc(str_lit_cpy_size);

	int idx = sprintf_s(str_lit_cpy, str_lit_cpy_size, "%s db ", str_name);

	str_lit_cpy[idx++] = '\"';

	char const * curr = str_lit;
	while (*curr) {
		if (*curr == '\\') {
			char escaped = *(curr + 1);

			switch (escaped) {
				case '0':  strcpy_s(str_lit_cpy + idx, str_lit_cpy_size - idx, "\", 0, \"");   idx += 7; break;
				case 'r':  strcpy_s(str_lit_cpy + idx, str_lit_cpy_size - idx, "\", 0Dh, \""); idx += 9; break;
				case 'n':  strcpy_s(str_lit_cpy + idx, str_lit_cpy_size - idx, "\", 0Ah, \""); idx += 9; break;
				case 't':  strcpy_s(str_lit_cpy + idx, str_lit_cpy_size - idx, "\", 09h, \""); idx += 9; break;

				case '\\': str_lit_cpy[idx++] = '\\'; break;

				default: {
					printf("ERROR: Invalid escape char '%c'!\n", escaped);
					error(ERROR_CODEGEN);
				}
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

	emit_data(emit, str_lit_cpy);
}

Condition_Code condition_code_invert(Condition_Code cc) {
	switch (cc) {
		case CC_E:  return CC_NE;
		case CC_NE: return CC_E;
		case CC_B:  return CC_AE;
		case CC_BE: return CC_A;
		case CC_A:  return CC_BE;
		case CC_AE: return CC_B;
		case CC_L:  return CC_GE;
		case CC_LE: return CC_G;
		case CC_G:  return CC_LE;
		case CC_GE: return CC_L;
		case CC_Z:  return CC_NZ;
		case CC_NZ: return CC_Z;
		case CC_S:  return CC_NS;
		case CC_NS: return CC_S;
		default: error(ERROR_INTERNAL);
	}
}

char const * condition_code_to_str(Condition_Code cc) {
	switch (cc) {
		case CC_E:  return "e";
		case CC_NE: return "ne";
		case CC_B:  return "b";
		case CC_BE: return "be";
		case CC_A:  return "a";
		case CC_AE: return "ae";
		case CC_L:  return "l";
		case CC_LE: return "le";
		case CC_G:  return "g";
		case CC_GE: return "ge";
		case CC_Z:  return "z";
		case CC_NZ: return "nz";
		case CC_S:  return "s";
		case CC_NS: return "ns";
		default: error(ERROR_INTERNAL);
	}
}

Result result_make_reg(Type const * type, Register reg) { return (Result) { .form = RESULT_REGISTER,  .by_address = false, .type = type, .reg  = reg }; }
Result result_make_i64(Type const * type, int64_t  i64) { return (Result) { .form = RESULT_IMMEDIATE, .by_address = false, .type = type, .i64  = i64 }; }
Result result_make_u64(Type const * type, uint64_t u64) { return (Result) { .form = RESULT_IMMEDIATE, .by_address = false, .type = type, .u64  = u64 }; }
Result result_make_f32(Type const * type, float    f32) { return (Result) { .form = RESULT_IMMEDIATE, .by_address = false, .type = type, .f32  = f32 }; }
Result result_make_f64(Type const * type, double   f64) { return (Result) { .form = RESULT_IMMEDIATE, .by_address = false, .type = type, .f64  = f64 }; }

Result result_make_cmp(Type const * type, Condition_Code cc) { return (Result) { .form = RESULT_COMPARE, .by_address = false, .type = type, .cc = cc }; }

Result result_make_sib(Type const * type, Register base, SIB_Scale scale, Register index, int32_t disp) {
	return (Result) { .form = RESULT_SIB, .by_address = false, .type = type, .sib = { base, scale, index, disp } };
}

Result result_make_global(Type const * type, char const * name) {
	return (Result) { .form = RESULT_GLOBAL, .by_address = false, .type = type, .global = { name, 0 } };
}

void result_free(Code_Emitter * emit, Result * result) {
	if (result->form == RESULT_REGISTER) {
		register_free(emit, result->reg);
	}
}

bool result_is_indirect(Result const * result) {
	return result->form == RESULT_SIB || result->form == RESULT_GLOBAL;
}

Result variable_get_address(Variable const * var) {
	if (var->is_global) {
		return result_make_global(var->type, var->name);
	} else {
		return result_make_sib(var->type, RBP, 0, 0, var->offset); // Locals / Arguments relative to stack frame pointer
	}
}

Condition_Code get_condition_code(Operator_Bin operator, Result const * lhs, Result const * rhs) {
	bool both_signed_ints = type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type);

	switch (operator)	{
		case OPERATOR_BIN_LT: if (both_signed_ints) return CC_L;  else return CC_B;
		case OPERATOR_BIN_LE: if (both_signed_ints) return CC_LE; else return CC_BE;
		case OPERATOR_BIN_GT: if (both_signed_ints) return CC_G;  else return CC_A;
		case OPERATOR_BIN_GE: if (both_signed_ints) return CC_GE; else return CC_AE;
		case OPERATOR_BIN_EQ: return CC_E;
		case OPERATOR_BIN_NE: return CC_NE;
	}
	error(ERROR_INTERNAL);
}

Condition_Code result_get_condition_code(Code_Emitter * emit, Result * result) {
	if (result->form == RESULT_COMPARE) {
		return result->cc;
	} else {
		emit_test(emit, result, result);
		return CC_NZ;
	}
}

void result_ensure_in_given_register(Code_Emitter * emit, Result * result, Register given_register) {
	if (result->form == RESULT_REGISTER && result->reg == given_register) return;

	flag_set(&emit->reg_mask, 1 << given_register);

	Result result_dst = result_make_reg(result->type, given_register);

	if (result->form == RESULT_COMPARE) {	
		emit_setcc(emit, result->cc, &result_dst);
		assert(type_is_bool(result->type));
	} else if (result->by_address) {
		emit_lea(emit, &result_dst, result);
	} else {
		emit_mov(emit, &result_dst, result);
	}

	result_free(emit, result);
	*result = result_dst;
}

void result_ensure_in_register(Code_Emitter * emit, Result * result) {
	if (result->form == RESULT_REGISTER) return;
	
	Register reg = (type_is_float(result->type) && !result->by_address) ?
		register_alloc_float(emit) :
		register_alloc(emit);
	result_ensure_in_given_register(emit, result, reg);
}

Result result_deref(Code_Emitter * emit, Result * address, bool allow_same_register) {
	Register reg;
	if (type_is_float(address->type)) {
		reg = register_alloc_float(emit);
	} else if (allow_same_register && address->form == RESULT_REGISTER && register_is_scratch(address->reg)) {
		reg = address->reg;
	} else {
		reg = register_alloc(emit);
	}

	Result result = result_make_reg(address->type, reg);

	if (address->by_address) {
		emit_lea(emit, &result, address);
		return result;
	}

	switch (address->form) {
		case RESULT_IMMEDIATE: error(ERROR_CODEGEN);
		case RESULT_REGISTER: {
			int reg_size = 8;
			int result_size = type_get_size(address->type, emit->current_scope);

			// Select correct mov instruction based on Type
			char const * mov = "mov";
			if (type_is_f32(address->type)) {
				mov = "movss";
			} else if (type_is_f64(address->type)) {
				mov = "movsd";
			} else if (result_size < 8) {
				if (type_is_integral_signed(address->type)) {
					mov = "movsx"; // Signed extend
				} else if (result_size < 4) {
					mov = "movzx"; // Zero extend
				} else {
					reg_size = 4; // 32bit movs will automatically zero extend the highest 32 bits
				}
			}

			char str_result [RESULT_STR_BUF_SIZE]; result_to_str_sized(str_result,  sizeof(str_result),  &result, reg_size);
			char str_address[RESULT_STR_BUF_SIZE]; result_to_str      (str_address, sizeof(str_address), address);
			emit_asm(emit, "%s %s, %s [%s]\n", mov, str_result, get_word_name(result_size), str_address);

			if (result.reg != address->reg) result_free(emit, address);
			return result;
		}
		case RESULT_SIB:
		case RESULT_GLOBAL: {		
			emit_mov(emit, &result, address);
			return result;
		}
		default: error(ERROR_INTERNAL);
	}
}

void result_ensure_fits_in_imm32(Code_Emitter * emit, Result * result) {
	assert(result->form == RESULT_IMMEDIATE);

	// Move to register if the immediate value is too large to fit in a DWORD
	if (result->i64 < INT_MIN || result->i64 > INT_MAX) {
		result_ensure_in_register(emit, result);
	}
}

void result_to_str_sized(char * str, int str_size, Result const * result, int size) {
	switch (result->form) {
		case RESULT_REGISTER: {
			strcpy_s(str, str_size, get_reg_name(result->reg, size));
			break;
		}
		case RESULT_IMMEDIATE: {
			if (type_is_integral(result->type) || type_is_pointer(result->type) || type_is_bool(result->type)) {
				sprintf_s(str, str_size, "%lli", result->i64); 
			} else if (type_is_f32(result->type)) {
				sprintf_s(str, str_size, "%f", result->f32);
			} else if (type_is_f64(result->type)) {
				sprintf_s(str, str_size, "%f", result->f64);
			} else {
				error(ERROR_INTERNAL);
			}
			break;
		}
		case RESULT_SIB: {
			int bytes_written = 0;

			bytes_written += sprintf_s(str + bytes_written, str_size - bytes_written, "%s [", get_word_name(size));
			bytes_written += sprintf_s(str + bytes_written, str_size - bytes_written, "%s", get_reg_name(result->sib.base, 8));
			if (result->sib.scale != SIB_SCALE_0) {
				bytes_written += sprintf_s(str + bytes_written, str_size - bytes_written, " + %i*%s", result->sib.scale, get_reg_name(result->sib.index, 8));
			}
			if (result->sib.disp != 0) {
				bytes_written += sprintf_s(str + bytes_written, str_size - bytes_written, " + %i", result->sib.disp);
			}
			bytes_written += sprintf_s(str + bytes_written, str_size - bytes_written, "]");

			break;
		}
		case RESULT_GLOBAL: {
			if (result->global.disp != 0) {
				sprintf_s(str, str_size, "%s [REL %s + %i]", get_word_name(size), result->global.name, result->global.disp);		
			} else { 
				sprintf_s(str, str_size, "%s [REL %s]", get_word_name(size), result->global.name);
			}
			break;
		}
		default: error(ERROR_INTERNAL);
	}
}

void result_to_str(char * str, int str_size, Result const * result) {
	result_to_str_sized(str, str_size, result, 8);
}


void emit_lea(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form != RESULT_REGISTER || !result_is_indirect(rhs)) {
		error(ERROR_CODEGEN);
	}

	char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str(str_lhs, sizeof(str_lhs), lhs);
	char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str(str_rhs, sizeof(str_rhs), rhs);
	emit_asm(emit, "lea %s, %s\n", str_lhs, str_rhs);
}

void emit_mov(Code_Emitter * emit, Result * lhs, Result * rhs) {
	assert(lhs->form != RESULT_COMPARE);

	if (rhs->form == RESULT_COMPARE) result_ensure_in_register(emit, rhs);

	int result_size = type_get_size(rhs->type, emit->current_scope);

	switch (lhs->form) {
		case RESULT_IMMEDIATE: error(ERROR_INTERNAL);
		case RESULT_REGISTER: {
			if (lhs->form == RESULT_REGISTER && rhs->form == RESULT_IMMEDIATE && rhs->u64 == 0) {
				emit_xor(emit, lhs, lhs);
				return;
			}

			if (rhs->by_address && result_is_indirect(rhs)) {
				emit_lea(emit, lhs, rhs);
				return;
			}

			int reg_size = 8;
			char const * mov = "mov";
			if (rhs->form == RESULT_IMMEDIATE) {
				if (result_size < 8 && type_is_integral_unsigned(rhs->type)) {
					reg_size = 4;
				}
			} else {
				if (type_is_float(lhs->type) ^ type_is_float(rhs->type)) { // Check if one operand is floating point but the other is not
					if (type_is_f32(lhs->type) || type_is_f32(rhs->type)) {
						mov = "movd";
						reg_size = 4;
					} else {
						mov = "movq";
					}
				} else if (type_is_f32(rhs->type)) {
					mov = "movss";
				} else if (type_is_f64(rhs->type)) {
					mov = "movsd";
				} else if(result_size < 8) {
					if (type_is_integral_signed(rhs->type)) {
						mov = "movsx"; // Signed extend
					} else if (result_size < 4) {
						mov = "movzx"; // Zero extend
					} else {
						reg_size = 4; // 32bit movs will automatically zero extend the highest 32 bits
					}
				}
			}

			char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
			char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, result_size);
			emit_asm(emit, "%s %s, %s\n", mov, str_lhs, str_rhs);
			break;
		}
		case RESULT_SIB:
		case RESULT_GLOBAL: {
			if (result_is_indirect(rhs)) {
				*rhs = result_deref(emit, rhs, true);
			}
			char const * mov = "mov";
			if (type_is_f32(rhs->type)) {
				mov = "movss";
			} else if (type_is_f64(rhs->type)) {
				mov = "movsd";
			}
			char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, result_size);
			char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, result_size);
			emit_asm(emit, "%s %s, %s\n", mov, str_lhs, str_rhs);
			break;
		}
	}
}

void emit_mov_indirect(Code_Emitter * emit, Result * lhs, Result * rhs) {
	assert(lhs->form != RESULT_COMPARE);
	if (rhs->form == RESULT_COMPARE) result_ensure_in_register(emit, rhs);

	char const * mov = "mov";
	if (type_is_f32(rhs->type)) {
		mov = "movss";
	} else if (type_is_f64(rhs->type)) {
		mov = "movsd";
	}
	
	int size_left  = 8;
	int size_right = type_get_size(lhs->type, emit->current_scope);

	if (result_is_indirect(lhs)) {
		size_left = size_right;
	}

	if (result_is_indirect(rhs)) {
		*rhs = result_deref(emit, rhs, true);
	}

	char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, size_left);
	char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, size_right);

	switch (lhs->form) {
		case RESULT_IMMEDIATE: error(ERROR_INTERNAL);
		case RESULT_REGISTER: {
			emit_asm(emit, "%s %s [%s], %s\n", mov, get_word_name(size_right), str_lhs, str_rhs);
			break;
		}
		case RESULT_SIB:
		case RESULT_GLOBAL: {			
			emit_asm(emit, "%s %s, %s\n", mov, str_lhs, str_rhs);
			break;
		}
	}
}

void emit_label(Code_Emitter * emit, char const * label) {
	emit_asm(emit, "%s:\n", label);
}

void emit_jmp(Code_Emitter * emit, char const * label) {
	emit_asm(emit, "jmp %s\n", label);
}

void emit_add(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		if (type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type)) {
			lhs->i64 += rhs->i64;
		} else if (type_is_integral(lhs->type) && type_is_integral(rhs->type)) {
			lhs->u64 += rhs->u64;
		} else if (type_is_f64(lhs->type) && type_is_f64(rhs->type)) {
			lhs->f64 += rhs->f64;
		} else if (type_is_f32(lhs->type) && type_is_f32(rhs->type)) {
			lhs->f32 += rhs->f32;
		} else {
			error(ERROR_TYPECHECK);
		}
	} else {
		if (lhs->form == RESULT_IMMEDIATE) {
			swap(lhs, rhs, sizeof(Result));
		}
		if (rhs->form == RESULT_IMMEDIATE) {
			result_ensure_fits_in_imm32(emit, rhs);
		}

		if (rhs->form == RESULT_IMMEDIATE && rhs->u64 == 0) {
			return;
		}

		if (lhs->by_address) {
			if (lhs->form == RESULT_GLOBAL && rhs->form == RESULT_IMMEDIATE) {
				lhs->global.disp += (int)rhs->i64;
				return;
			} else if (lhs->form == RESULT_SIB && rhs->form == RESULT_IMMEDIATE) {
				lhs->sib.disp += (int)rhs->i64;
				return;
			}
		}

		result_ensure_in_register(emit, lhs);

		// If the rhs is 32 bit, the upper 32 bits of the lhs will be zeroed. To avoid this, first put the rhs in a 64 bit register
		if (type_get_size(lhs->type, emit->current_scope) == 8 &&
			type_get_size(rhs->type, emit->current_scope) == 4) {
			result_ensure_in_register(emit, rhs);
		}

		int reg_size = result_is_indirect(rhs) ? type_get_size(rhs->type, emit->current_scope) : 8;

		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, reg_size);

		if (type_is_f32(lhs->type)) {
			emit_asm(emit, "addss %s, %s\n", str_lhs, str_rhs);
		} else if (type_is_f64(rhs->type)) {
			emit_asm(emit, "addsd %s, %s\n", str_lhs, str_rhs);
		} else if (rhs->form == RESULT_IMMEDIATE && rhs->i64 == 1) {
				emit_asm(emit, "inc %s\n", str_lhs);
		} else if (rhs->form == RESULT_IMMEDIATE && rhs->i64 == -1) {
			emit_asm(emit, "dec %s\n", str_lhs);
		} else {
			emit_asm(emit, "add %s, %s\n", str_lhs, str_rhs);
		}
	}
}

void emit_sub(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		if (type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type)) {
			lhs->i64 -= rhs->i64;
		} else if (type_is_integral(lhs->type) && type_is_integral(rhs->type)) {
			lhs->u64 -= rhs->u64;
		} else if (type_is_f64(lhs->type) && type_is_f64(rhs->type)) {
			lhs->f64 -= rhs->f64;
		} else if (type_is_f32(lhs->type) && type_is_f32(rhs->type)) {
			lhs->f32 -= rhs->f32;
		} else {
			error(ERROR_TYPECHECK);
		}
	} else {
		if (rhs->form == RESULT_IMMEDIATE) {
			result_ensure_fits_in_imm32(emit, rhs);
		}
		
		if (rhs->form == RESULT_IMMEDIATE && rhs->u64 == 0) {
			return;
		}
		
		if (lhs->by_address) {
			if (lhs->form == RESULT_GLOBAL && rhs->form == RESULT_IMMEDIATE) {
				lhs->global.disp -= (int)rhs->i64;
				return;
			} else if (lhs->form == RESULT_SIB && rhs->form == RESULT_IMMEDIATE) {
				lhs->sib.disp -= (int)rhs->i64;
				return;
			}
		}

		result_ensure_in_register(emit, lhs);
		
		// If the rhs is 32 bit, the upper 32 bits of the lhs will be zeroed. To avoid this, first put the rhs in a 64 bit register
		if (type_get_size(lhs->type, emit->current_scope) == 8 &&
			type_get_size(rhs->type, emit->current_scope) == 4) {
			result_ensure_in_register(emit, rhs);
		}

		int reg_size = result_is_indirect(rhs) ? type_get_size(rhs->type, emit->current_scope) : 8;

		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, reg_size);

		if (type_is_f32(lhs->type)) {
			emit_asm(emit, "subss %s, %s\n", str_lhs, str_rhs);
		} else if (type_is_f64(rhs->type)) {
			emit_asm(emit, "subsd %s, %s\n", str_lhs, str_rhs);
		} else if (rhs->form == RESULT_IMMEDIATE && rhs->i64 == 1) {
			emit_asm(emit, "dec %s\n", str_lhs);
		} else if (rhs->form == RESULT_IMMEDIATE && rhs->i64 == -1) {
			emit_asm(emit, "inc %s\n", str_lhs);
		} else {
			emit_asm(emit, "sub %s, %s\n", str_lhs, str_rhs);
		}
	}
}

void emit_mul(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		if (type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type)) {
			lhs->i64 *= rhs->i64;
		} else if (type_is_integral(lhs->type) && type_is_integral(rhs->type)) {
			lhs->u64 *= rhs->u64;
		} else if (type_is_f64(lhs->type) && type_is_f64(rhs->type)) {
			lhs->f64 *= rhs->f64;
		} else if (type_is_f32(lhs->type) && type_is_f32(rhs->type)) {
			lhs->f32 *= rhs->f32;
		} else {
			error(ERROR_TYPECHECK);
		}
	} else {
		if (lhs->form == RESULT_IMMEDIATE) {
			swap(lhs, rhs, sizeof(Result));
		}
		if (rhs->form == RESULT_IMMEDIATE) {
			result_ensure_fits_in_imm32(emit, rhs);
		}

		if (type_is_integral(rhs->type) && rhs->form == RESULT_IMMEDIATE) {
			if (rhs->u64 == 1) {
				return;
			} else if (rhs->u64 == 3 || rhs->u64 == 5 || rhs->u64 == 9) {			
				result_ensure_in_register(emit, lhs);
				Result result_sib = result_make_sib(rhs->type, lhs->reg, rhs->u64 - 1, lhs->reg, 0);
				emit_lea(emit, lhs, &result_sib);
				return;
			} else if (is_power_of_two(rhs->u64)) {
				Result result_log = result_make_u64(rhs->type, log2(rhs->u64));
				emit_shift_left(emit, lhs, &result_log);
				result_free(emit, &result_log);
				return;
			} else if (lhs->form == RESULT_REGISTER && rhs->u64 == 0) {
				emit_xor(emit, lhs, lhs);
				return;
			} else {
				result_ensure_in_register(emit, lhs);
				char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str(str_lhs, sizeof(str_lhs), lhs);
				char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str(str_rhs, sizeof(str_rhs), rhs);
				emit_asm(emit, "imul %s, %s, %s\n", str_lhs, str_lhs, str_rhs);
				return;
			}
		}

		result_ensure_in_register(emit, lhs);
		
		// If the rhs is 32 bit, the upper 32 bits of the lhs will be zeroed. To avoid this, first put the rhs in a 64 bit register
		if (type_get_size(lhs->type, emit->current_scope) == 8 &&
			type_get_size(rhs->type, emit->current_scope) == 4) {
			result_ensure_in_register(emit, rhs);
		}

		int reg_size = result_is_indirect(rhs) ? type_get_size(rhs->type, emit->current_scope) : 8;

		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, reg_size);

		char const * instruction = "imul";
		if (type_is_f32(lhs->type)) {
			instruction = "mulss";
		} else if (type_is_f64(rhs->type)) {
			instruction = "mulsd";
		}
		emit_asm(emit, "%s %s, %s\n", instruction, str_lhs, str_rhs);
	}
}

void emit_div(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		if (type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type)) {
			lhs->i64 /= rhs->i64;
		} else if (type_is_integral(lhs->type) && type_is_integral(rhs->type)) {
			lhs->u64 /= rhs->u64;
		} else if (type_is_f64(lhs->type) && type_is_f64(rhs->type)) {
			lhs->f64 /= rhs->f64;
		} else if (type_is_f32(lhs->type) && type_is_f32(rhs->type)) {
			lhs->f32 /= rhs->f32;
		} else {
			error(ERROR_TYPECHECK);
		}
	} else {
		result_ensure_in_register(emit, lhs);

		if (type_is_integral(rhs->type) && rhs->form == RESULT_IMMEDIATE) {
			uint32_t const TWO_TO_THE_31 = 1u << 31;
			
			uint64_t divisor = rhs->u64;

			if (divisor == 0) {
				puts("ERROR: Integer division by 0!");
				error(ERROR_CODEGEN);
			} else if (divisor == 1) {
				return;
			} else if (is_power_of_two(divisor)) {
				assert(lhs->form == RESULT_REGISTER);
				Result result_bias = result_make_sib(rhs->type, lhs->reg, 0, 0, divisor - 1);
				Result result_reg  = result_make_reg(lhs->type, register_alloc(emit));
				emit_lea(emit, &result_reg, &result_bias);
				emit_test(emit, lhs, lhs);
				emit_cmovcc(emit, CC_NS, &result_reg, lhs);

				Result result_log  = result_make_u64(rhs->type, log2(divisor));
				emit_shift_right(emit, &result_reg, &result_log);

				result_free(emit, &result_bias);
				result_free(emit, &result_log);

				result_free(emit, lhs);
				*lhs = result_reg;
				return;
			} else if (divisor >= 2 && divisor < TWO_TO_THE_31) {
				/////////////////////////////////////////
				// Based on Hackers Delight chapter 10 //
				/////////////////////////////////////////

				uint32_t ad  = divisor;
				uint32_t t   = TWO_TO_THE_31 + (divisor >> 31);
				uint32_t anc = t - 1 - t % ad;

				uint32_t p  = 31;
				uint32_t q1 = TWO_TO_THE_31 / anc;
				uint32_t r1 = TWO_TO_THE_31 - q1 * anc;
				uint32_t q2 = TWO_TO_THE_31 / ad;
				uint32_t r2 = TWO_TO_THE_31 - q2 * ad;
				uint32_t delta = 0;

				do {      
					p = p + 1;      
					q1 = 2 * q1;
					r1 = 2 * r1;
					if (r1 >= anc) {
						q1 = q1 + 1;
						r1 = r1 - anc;
					}      
					q2 = 2 * q2;
					r2 = 2 * r2;
					if (r2 >= ad) {
						q2 = q2 + 1;
						r2 = r2 - ad;
					}      
					delta = ad - r2;   
				} while (q1 < delta || (q1 == delta && r1 == 0));

				Result result_magic      = result_make_u64(make_type_u64(), q2 + 1);
				Result result_shift      = result_make_u64(make_type_u64(), p);
				Result result_sign       = result_make_reg(make_type_u64(), register_alloc(emit));
				Result result_sign_shift = result_make_u64(make_type_u64(), 63);

				Type const * type = lhs->type;
				lhs->type = make_type_i64();

				emit_mul(emit, lhs, &result_magic);
				emit_mov(emit, &result_sign, lhs);
				emit_shift_right(emit, &result_sign, &result_sign_shift);
				emit_shift_right(emit, lhs, &result_shift);
				emit_add(emit, lhs, &result_sign);

				result_free(emit, &result_magic);
				result_free(emit, &result_shift);
				result_free(emit, &result_sign);
				result_free(emit, &result_sign_shift);

				lhs->type = type;

				return;
			}
		}

		result_ensure_in_register(emit, rhs);
		
		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str(str_lhs, sizeof(str_lhs), lhs);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str(str_rhs, sizeof(str_rhs), rhs);

		if (type_is_f32(lhs->type)) {
			emit_asm(emit, "divss %s, %s\n", str_lhs, str_rhs);
		} else if (type_is_f64(rhs->type)) {
			emit_asm(emit, "divsd %s, %s\n", str_lhs, str_rhs);
		} else {
			emit_asm(emit, "mov rax, %s\n", str_lhs);
			emit_asm(emit, "cqo\n");
			emit_asm(emit, "idiv %s\n",     str_rhs);
			emit_asm(emit, "mov %s, rax\n", str_lhs);
		}
	}
}

void emit_mod(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		lhs->u64 %= rhs->u64;
	} else {
		result_ensure_in_register(emit, lhs);
		result_ensure_in_register(emit, rhs);
		
		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str(str_lhs, sizeof(str_lhs), lhs);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str(str_rhs, sizeof(str_rhs), rhs);

		emit_asm(emit, "mov rax, %s\n", str_lhs);
		emit_asm(emit, "cqo\n");
		emit_asm(emit, "idiv %s\n",     str_rhs);
		emit_asm(emit, "mov %s, rdx\n", str_lhs);
	}
}

void emit_shift_left(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		if (type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type)) {
			lhs->i64 <<= rhs->i64;
		} else if (type_is_integral(lhs->type) && type_is_integral(rhs->type)) {
			lhs->u64 <<= rhs->u64;
		} else {
			error(ERROR_TYPECHECK);
		}
	} else {
		result_ensure_in_register(emit, lhs);
		
		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str(str_lhs, sizeof(str_lhs), lhs);
		
		if (rhs->form == RESULT_IMMEDIATE) {
			if (rhs->u64 != 0) {
				emit_asm(emit, "shl %s, %llu\n", str_lhs, rhs->u64);
			}
		} else {
			result_ensure_in_given_register(emit, rhs, RCX);
			emit_asm(emit, "shl %s, cl\n", str_lhs);
		}
	}
}

void emit_shift_right(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		if (type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type)) {
			lhs->i64 >>= rhs->i64;
		} else if (type_is_integral(lhs->type) && type_is_integral(rhs->type)) {
			lhs->u64 >>= rhs->u64;
		} else {
			error(ERROR_TYPECHECK);
		}
	} else {
		result_ensure_in_register(emit, lhs);
		
		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str(str_lhs, sizeof(str_lhs), lhs);
		char const * instruction = type_is_integral_signed(lhs->type) ? "sar" : "shr";

		if (rhs->form == RESULT_IMMEDIATE) {
			if (rhs->u64 != 0) {
				emit_asm(emit, "%s %s, %llu\n", instruction, str_lhs, rhs->u64);
			}
		} else {
			result_ensure_in_given_register(emit, rhs, RCX);
			emit_asm(emit, "%s %s, cl\n", instruction, str_lhs);
		}
	}
}

void emit_and(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		lhs->u64 &= rhs->u64;
	} else {
		if (lhs->form == RESULT_IMMEDIATE) {
			swap(lhs, rhs, sizeof(Result));
		}
		if (rhs->form == RESULT_IMMEDIATE) {
			result_ensure_fits_in_imm32(emit, rhs);
		}

		result_ensure_in_register(emit, lhs);
		
		// If the rhs is 32 bit, the upper 32 bits of the lhs will be zeroed. To avoid this, first put the rhs in a 64 bit register
		if (type_get_size(lhs->type, emit->current_scope) == 8 &&
			type_get_size(rhs->type, emit->current_scope) == 4) {
			result_ensure_in_register(emit, rhs);
		}

		int reg_size = result_is_indirect(rhs) ? type_get_size(rhs->type, emit->current_scope) : 8;

		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, reg_size);

		emit_asm(emit, "and %s, %s\n", str_lhs, str_rhs);
	}
}

void emit_xor(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		lhs->u64 ^= rhs->u64;
	} else {
		if (lhs->form == RESULT_IMMEDIATE) {
			swap(lhs, rhs, sizeof(Result));
		}
		if (rhs->form == RESULT_IMMEDIATE) {
			result_ensure_fits_in_imm32(emit, rhs);
		}

		result_ensure_in_register(emit, lhs);
		
		// If the rhs is 32 bit, the upper 32 bits of the lhs will be zeroed. To avoid this, first put the rhs in a 64 bit register
		if (type_get_size(lhs->type, emit->current_scope) == 8 &&
			type_get_size(rhs->type, emit->current_scope) == 4) {
			result_ensure_in_register(emit, rhs);
		}

		int reg_size = result_is_indirect(rhs) ? type_get_size(rhs->type, emit->current_scope) : 8;

		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, reg_size);

		emit_asm(emit, "xor %s, %s\n", str_lhs, str_rhs);
	}
}

void emit_or(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		lhs->u64 |= rhs->u64;
	} else {
		if (lhs->form == RESULT_IMMEDIATE) {
			swap(lhs, rhs, sizeof(Result));
		}
		if (rhs->form == RESULT_IMMEDIATE) {
			result_ensure_fits_in_imm32(emit, rhs);
		}

		result_ensure_in_register(emit, lhs);
		
		// If the rhs is 32 bit, the upper 32 bits of the lhs will be zeroed. To avoid this, first put the rhs in a 64 bit register
		if (type_get_size(lhs->type, emit->current_scope) == 8 &&
			type_get_size(rhs->type, emit->current_scope) == 4) {
			result_ensure_in_register(emit, rhs);
		}

		int reg_size = result_is_indirect(rhs) ? type_get_size(rhs->type, emit->current_scope) : 8;

		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, reg_size);

		emit_asm(emit, "or %s, %s\n", str_lhs, str_rhs);
	}
}

void emit_neg(Code_Emitter * emit, Result * result) {
	if (result->form == RESULT_IMMEDIATE) {
		if (type_is_f32(result->type)) {
			result->f32 = -result->f32;
		} else if (type_is_f64(result->type)) {
			result->f64 = -result->f64;		
		} else {
			result->i64 = -result->i64;
		}
	} else {
		int type_size = type_get_size(result->type, emit->current_scope);

		Result result_tmp = result_make_reg(type_size == 4 ? make_type_u32() : make_type_u64(), register_alloc(emit));

		if (type_is_f32(result->type)) {
			// X-OR sign bit
			Result result_imm_2_to_the_31 = result_make_u64(make_type_u32(), 1ull << 31);

			emit_mov(emit, &result_tmp, result);
			emit_xor(emit, &result_tmp, &result_imm_2_to_the_31);
			emit_mov(emit, result, &result_tmp);
		} else if (type_is_f64(result->type)) {
			// X-OR sign bit using register (cannot xor with imm64)
			Result result_imm_2_to_the_63 = result_make_u64(make_type_u64(), 1ull << 63);
			Result result_reg_2_to_the_63 = result_make_reg(make_type_u64(), register_alloc(emit));

			emit_mov(emit, &result_tmp, result);
			emit_mov(emit, &result_reg_2_to_the_63, &result_imm_2_to_the_63);
			emit_xor(emit, &result_tmp, &result_reg_2_to_the_63);
			emit_mov(emit, result, &result_tmp);

			result_free(emit, &result_reg_2_to_the_63);
		} else {
			char str_result[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_result, sizeof(str_result), result, type_size);
			emit_asm(emit, "neg %s\n", str_result);
		}

		result_free(emit, &result_tmp);
	}
}

void emit_not(Code_Emitter * emit, Result * result) {
	if (result->form == RESULT_IMMEDIATE) {
		result->u64 = ~result->u64;
	} else {
		result_ensure_in_register(emit, result);
		
		char str_result[RESULT_STR_BUF_SIZE]; result_to_str(str_result, sizeof(str_result), result);
		emit_asm(emit, "not %s\n", str_result);
	}
}

void emit_test(Code_Emitter * emit, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		lhs->u64 &= rhs->u64;
	} else {
		if (lhs->form == RESULT_IMMEDIATE) {
			swap(lhs, rhs, sizeof(Result));
		}
		if (rhs->form == RESULT_IMMEDIATE) {
			result_ensure_fits_in_imm32(emit, rhs);
		}

		result_ensure_in_register(emit, lhs);
		
		// If the rhs is 32 bit, the upper 32 bits of the lhs will be zeroed. To avoid this, first put the rhs in a 64 bit register
		if (type_get_size(lhs->type, emit->current_scope) == 8 &&
			type_get_size(rhs->type, emit->current_scope) == 4) {
			result_ensure_in_register(emit, rhs);
		}

		int reg_size = result_is_indirect(rhs) ? type_get_size(rhs->type, emit->current_scope) : 8;

		char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_lhs, sizeof(str_lhs), lhs, reg_size);
		char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_rhs, sizeof(str_rhs), rhs, reg_size);

		emit_asm(emit, "test %s, %s\n", str_lhs, str_rhs);
	}
}

void emit_cmp(Code_Emitter * emit, Operator_Bin operator, Result * lhs, Result * rhs) {
	if (lhs->form == RESULT_IMMEDIATE && rhs->form == RESULT_IMMEDIATE) {
		bool compare_result = false;

		if (type_is_f32(lhs->type)) {
			switch (operator) {
				case OPERATOR_BIN_LT: compare_result = lhs->f32 <  rhs->f32; break;
				case OPERATOR_BIN_LE: compare_result = lhs->f32 <= rhs->f32; break;
				case OPERATOR_BIN_GT: compare_result = lhs->f32 >  rhs->f32; break;
				case OPERATOR_BIN_GE: compare_result = lhs->f32 >= rhs->f32; break;
				case OPERATOR_BIN_EQ: compare_result = lhs->f32 == rhs->f32; break;
				case OPERATOR_BIN_NE: compare_result = lhs->f32 != rhs->f32; break;
				default: error(ERROR_INTERNAL);
			}
		} else if (type_is_f64(lhs->type)) {
			switch (operator) {
				case OPERATOR_BIN_LT: compare_result = lhs->f64 <  rhs->f64; break;
				case OPERATOR_BIN_LE: compare_result = lhs->f64 <= rhs->f64; break;
				case OPERATOR_BIN_GT: compare_result = lhs->f64 >  rhs->f64; break;
				case OPERATOR_BIN_GE: compare_result = lhs->f64 >= rhs->f64; break;
				case OPERATOR_BIN_EQ: compare_result = lhs->f64 == rhs->f64; break;
				case OPERATOR_BIN_NE: compare_result = lhs->f64 != rhs->f64; break;
				default: error(ERROR_INTERNAL);
			}
		} else if (type_is_integral_signed(lhs->type) && type_is_integral_signed(rhs->type)) {
			switch (operator) {
				case OPERATOR_BIN_LT: compare_result = lhs->i64 <  rhs->i64; break;
				case OPERATOR_BIN_LE: compare_result = lhs->i64 <= rhs->i64; break;
				case OPERATOR_BIN_GT: compare_result = lhs->i64 >  rhs->i64; break;
				case OPERATOR_BIN_GE: compare_result = lhs->i64 >= rhs->i64; break;
				case OPERATOR_BIN_EQ: compare_result = lhs->i64 == rhs->i64; break;
				case OPERATOR_BIN_NE: compare_result = lhs->i64 != rhs->i64; break;
				default: error(ERROR_INTERNAL);
			}
		} else {
			switch (operator) {
				case OPERATOR_BIN_LT: compare_result = lhs->u64 <  rhs->u64; break;
				case OPERATOR_BIN_LE: compare_result = lhs->u64 <= rhs->u64; break;
				case OPERATOR_BIN_GT: compare_result = lhs->u64 >  rhs->u64; break;
				case OPERATOR_BIN_GE: compare_result = lhs->u64 >= rhs->u64; break;
				case OPERATOR_BIN_EQ: compare_result = lhs->u64 == rhs->u64; break;
				case OPERATOR_BIN_NE: compare_result = lhs->u64 != rhs->u64; break;
				default: error(ERROR_INTERNAL);
			}
		}

		result_free(emit, lhs);
		*lhs = result_make_u64(make_type_bool(), compare_result);
		return;
	}
	
	Operator_Bin original_operator = operator;
	
	if (lhs->form == RESULT_IMMEDIATE) {
		// Change operator to account for swapping of operands
		switch (operator) {
			case OPERATOR_BIN_LT: operator = OPERATOR_BIN_GT; break; // <  becomes >
			case OPERATOR_BIN_LE: operator = OPERATOR_BIN_GE; break; // <= becomes >=
			case OPERATOR_BIN_GT: operator = OPERATOR_BIN_LT; break; // >  becomes <
			case OPERATOR_BIN_GE: operator = OPERATOR_BIN_LE; break; // >= becomes <=
			case OPERATOR_BIN_EQ: break;                             // == stays ==
			case OPERATOR_BIN_NE: break;                             // != stays !=
			default: break;
		}
		swap(lhs, rhs, sizeof(Result));
	}
	if (rhs->form == RESULT_IMMEDIATE) {
		result_ensure_fits_in_imm32(emit, rhs);
	}

	result_ensure_in_register(emit, lhs);
	
	char const * cmp = NULL;
	if ((type_is_integral(lhs->type) && type_is_integral(rhs->type)) ||
		(type_is_bool    (lhs->type) && type_is_bool    (rhs->type)) ||
		(type_is_pointer (lhs->type) && type_is_pointer (rhs->type) && types_unifiable(lhs->type, rhs->type))
	) {
		cmp = "cmp";
	} else if (type_is_f32(lhs->type) && type_is_f32(rhs->type)) {
		cmp = "comiss";
	} else if (type_is_f64(lhs->type) && type_is_f64(rhs->type)) {
		cmp = "comisd";
	} else {
		type_error(emit, "Operator '%s' requires two integral, boolean, float, or pointer types", operator_bin_to_str(original_operator));
	}

	int type_size = MAX(
		type_get_size(lhs->type, emit->current_scope),
		type_get_size(rhs->type, emit->current_scope)
	);

	char str_result_left [RESULT_STR_BUF_SIZE]; result_to_str_sized(str_result_left, sizeof(str_result_left),  lhs, type_size);
	char str_result_right[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_result_right,sizeof(str_result_right), rhs, type_size);
	emit_asm(emit, "%s %s, %s\n", cmp, str_result_left, str_result_right);

	result_free(emit, lhs);
	*lhs = result_make_cmp(make_type_bool(), get_condition_code(operator, lhs, rhs));
}

void emit_jcc(Code_Emitter * emit, Condition_Code cc, char const * label) {
	emit_asm(emit, "j%s %s\n", condition_code_to_str(cc), label);
}

void emit_setcc(Code_Emitter * emit, Condition_Code cc, Result * result) {
	if (result->form != RESULT_REGISTER) {
		error(ERROR_CODEGEN);
	}

	char str_result[RESULT_STR_BUF_SIZE]; result_to_str_sized(str_result, sizeof(str_result), result, 1);
	emit_asm(emit, "set%s %s\n", condition_code_to_str(cc), str_result);
}

void emit_cmovcc(Code_Emitter * emit, Condition_Code cc, Result * lhs, Result * rhs) {
	result_ensure_in_register(emit, lhs);
	if (rhs->form == RESULT_IMMEDIATE) {
		result_ensure_fits_in_imm32(emit, rhs);
	}

	char str_lhs[RESULT_STR_BUF_SIZE]; result_to_str(str_lhs, sizeof(str_lhs), lhs);
	char str_rhs[RESULT_STR_BUF_SIZE]; result_to_str(str_rhs, sizeof(str_rhs), rhs);
	emit_asm(emit, "cmov%s %s, %s\n", condition_code_to_str(cc), str_lhs, str_rhs);
}

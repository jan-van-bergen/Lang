#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "AST.h"
#include "Scope.h"

#include "Util.h"

typedef enum Register {
	RAX,
	RBX,
	RCX,
	RDX,
	RSP,
	RBP,
	RSI,
	RDI,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,

	XMM0,
	XMM1,
	XMM2,
	XMM3,
	XMM4,
	XMM5,
	XMM6,
	XMM7,
	XMM8,
	XMM9,
	XMM10,
	XMM11,
	XMM12,
	XMM13,
	XMM14,
	XMM15,
} Register;

typedef uint32_t regmask_t;

bool register_is_float  (Register reg);
bool register_is_scratch(Register reg);

Register register_alloc      (struct Code_Emitter * emit);
Register register_alloc_float(struct Code_Emitter * emit);

bool register_is_reserved(struct Code_Emitter * emit, Register reg);
void register_free       (struct Code_Emitter * emit, Register reg);

Register get_call_register(int argument_index, bool is_float);

char const * get_reg_name(Register reg, int size);
char const * get_word_name(int size);


typedef enum SIB_Scale {
	SIB_SCALE_0 = 0,
	SIB_SCALE_1 = 1,
	SIB_SCALE_2 = 2,
	SIB_SCALE_4 = 4,
	SIB_SCALE_8 = 8
} SIB_Scale;


typedef enum Emit_Flags {
	EMIT_FLAG_EVAL_BY_ADDRESS  = 1,
	EMIT_FLAG_INSIDE_CONDITION = 2
} Emit_Flags;

typedef struct Trace_Element {
	enum Trace_Element_Type {
		TRACE_EXPRESSION,
		TRACE_STATEMENT
	} type;

	union {
		AST_Expression * expr;
		AST_Statement  * stat;
	};
} Trace_Element;

#define MAX_TRACE 64


typedef struct Code_Emitter {
	bool needs_main;
	bool emit_debug_lines;

	regmask_t reg_mask;

	int indent;

	int label;
	int current_loop_label;

	uint32_t flags;

	char const * current_function_name;
	char const * current_condition_label_true;
	char const * current_condition_label_false;
	Scope      * current_scope;

	char * code;
	int    code_len;
	int    code_cap;

	char const ** data_seg_vals;
	int           data_seg_len;
	int           data_seg_cap;
	
	char const ** bss;
	int           bss_len;
	int           bss_cap;

	int           trace_stack_size;
	Trace_Element trace_stack[MAX_TRACE];
} Code_Emitter;

Code_Emitter make_emit(bool needs_main, bool emit_debug_lines);

int get_new_label(Code_Emitter * emit);

void emit_asm (Code_Emitter * emit, char const * fmt, ...);
void emit_data(Code_Emitter * emit, char const * data);
void emit_bss (Code_Emitter * emit, char const * bss);

void emit_trace_push_expr(Code_Emitter * emit, AST_Expression * expr);
void emit_trace_push_stat(Code_Emitter * emit, AST_Statement  * stat);
void emit_trace_pop(Code_Emitter * emit);
void emit_print_stack_trace(Code_Emitter * emit);

NO_RETURN void type_error(Code_Emitter * emit, char const * msg, ...);

void emit_global(Code_Emitter * emit, Variable * var, bool sign, uint64_t value);
void emit_f32_literal(Code_Emitter * emit, char const * flt_name, float  value);
void emit_f64_literal(Code_Emitter * emit, char const * flt_name, double value);
void emit_string_literal(Code_Emitter * emit, char const * str_name, char const * str_lit);


typedef enum Condition_Code {
	CC_E,
	CC_NE,
	CC_B,
	CC_BE,
	CC_A,
	CC_AE,
	CC_L,
	CC_LE,
	CC_G,
	CC_GE,
	CC_Z,
	CC_NZ,
	CC_S,
	CC_NS
} Condition_Code;

Condition_Code condition_code_invert(Condition_Code cc);
char const *   condition_code_to_str(Condition_Code cc);

typedef enum Result_Form {
	RESULT_REGISTER,
	RESULT_IMMEDIATE,
	RESULT_COMPARE,
	RESULT_SIB,
	RESULT_GLOBAL
} Result_Form;

// Result of an AST_Expression
typedef struct Result {
	Result_Form form;
	bool by_address;
	Type const * type;
	union {
		Register reg;
		int64_t  i64;
		uint64_t u64;
		float    f32;
		double   f64;
		Condition_Code cc;
		struct {
			Register  base;
			SIB_Scale scale;
			Register  index;
			int       disp;
		} sib;
		struct {
			char const * name;
			int          disp;
		} global;
	};
} Result;

#define RESULT_STR_BUF_SIZE 64

Result result_make_reg   (Type const * type, Register reg);
Result result_make_i64   (Type const * type, int64_t  i64);
Result result_make_u64   (Type const * type, uint64_t u64);
Result result_make_f32   (Type const * type, float    f32);
Result result_make_f64   (Type const * type, double   f64);
Result result_make_cmp   (Type const * type, Condition_Code cc);
Result result_make_sib   (Type const * type, Register base, SIB_Scale scale, Register index, int32_t disp);
Result result_make_global(Type const * type, char const * name);

void result_free(Code_Emitter * emit, Result * result);

bool result_is_indirect(Result const * result);

Result variable_get_address(Variable const * var);

Condition_Code get_condition_code(Operator_Bin operator, Result const * lhs, Result const * rhs);

Condition_Code result_get_condition_code(Code_Emitter * emit, Result * result);

void result_ensure_in_given_register(Code_Emitter * emit, Result * result, Register given_register);
void result_ensure_in_register      (Code_Emitter * emit, Result * result);
void result_ensure_fits_in_imm32    (Code_Emitter * emit, Result * result);

Result result_deref(Code_Emitter * emit, Result * address, bool allow_same_register);

void result_to_str_sized(char * str, int str_size, Result const * result, int size);
void result_to_str      (char * str, int str_size, Result const * result);

void emit_lea         (Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_mov         (Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_mov_indirect(Code_Emitter * emit, Result * lhs, Result * rhs);

void emit_label(Code_Emitter * emit, char const * label);

void emit_jmp(Code_Emitter * emit, char const * label);

void emit_add(Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_sub(Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_mul(Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_div(Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_mod(Code_Emitter * emit, Result * lhs, Result * rhs);

void emit_shift_left (Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_shift_right(Code_Emitter * emit, Result * lhs, Result * rhs);

void emit_and(Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_xor(Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_or (Code_Emitter * emit, Result * lhs, Result * rhs);

void emit_neg(Code_Emitter * emit, Result * result);
void emit_not(Code_Emitter * emit, Result * result);

void emit_test(Code_Emitter * emit, Result * lhs, Result * rhs);
void emit_cmp (Code_Emitter * emit, Operator_Bin operator, Result * lhs, Result * rhs);

void emit_jcc   (Code_Emitter * emit, Condition_Code cc, char const * label);
void emit_setcc (Code_Emitter * emit, Condition_Code cc, Result * result);
void emit_cmovcc(Code_Emitter * emit, Condition_Code cc, Result * lhs, Result * rhs);

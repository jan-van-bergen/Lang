#include "Godegen.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#define RAX 0
#define RBX 1
#define RCX 2
#define RDX 3
#define R8  4
#define R9  5
#define R10 6
#define R11 7
#define R12 8
#define R13 9
#define R14 10
#define R15 11

#define REGISTER_COUNT 12

static char const * reg_names[REGISTER_COUNT] = {
	[RAX] = "rax",
	[RBX] = "rbx",
	[RCX] = "rcx",
	[RDX] = "rdx",
	[R8 ] = "r8",
	[R9 ] = "r9",
	[R10] = "r10",
	[R11] = "r11",
	[R12] = "r12",
	[R13] = "r13",
	[R14] = "r14",
	[R15] = "r15",
};
	
static char const * function_call_reg_names[] = { "rcx", "rdx", "r8", "r9" }; 

typedef struct Scope {
	struct Scope * prev;

	int           arg_count;
	char const ** args;

	int           var_count;
	char const ** vars;

	int curr_arg_offset;
	int curr_var_offset;

	int stack_frame_size;
} Scope;

typedef struct Context {
	bool regs_occupied[12];

	int indent;

	int label;
	int current_loop_label;

	Scope * current_scope;
	int stack_offset;

	char * code;
	int    code_len;
	int    code_cap;

	char const ** string_lits; // String Literals
	int           string_lit_len;
	int           string_lit_cap;
} Context;

void context_init(Context * ctx) {
	memset(ctx->regs_occupied, 0, sizeof(ctx->regs_occupied));

	ctx->indent = 0;

	ctx->label = 0;
	ctx->current_loop_label = -1;

	ctx->current_scope = NULL;
	ctx->stack_offset = 0;

	ctx->code_len = 0;
	ctx->code_cap = 512;
	ctx->code     = malloc(ctx->code_cap);

	ctx->string_lit_len = 0;
	ctx->string_lit_cap = 16;
	ctx->string_lits    = malloc(ctx->string_lit_cap * sizeof(char const *));
}

int context_reg_request(Context * context) {
	for (int i = 0; i < REGISTER_COUNT; i++) {
		if (i >= RCX && i <= R9) continue;

		if (!context->regs_occupied[i]) {
			context->regs_occupied[i] = true;

			return i;
		}
	}

	abort(); // No registers available!
}

void context_reg_free(Context * ctx, int reg) {
	if (reg == -1) return;

	assert(ctx->regs_occupied[reg]);

	ctx->regs_occupied[reg] = false;
}

int context_new_label(Context * ctx) {
	return ctx->label++;
}

Scope * context_scope_push(Context * ctx, int arg_count, int var_count) {
	Scope * scope = malloc(sizeof(Scope));
	scope->prev = ctx->current_scope;

	scope->arg_count = arg_count;
	scope->var_count = var_count;

	scope->args = calloc(arg_count, sizeof(char const *));
	scope->vars = calloc(var_count, sizeof(char const *));

	scope->curr_arg_offset = 0;
	scope->curr_var_offset = 0;

	scope->stack_frame_size = 0;

	ctx->current_scope = scope;

	return scope;
}

void context_scope_pop(Context * ctx) {
	Scope * scope = ctx->current_scope;
	ctx->current_scope = scope->prev;

	free(scope->vars);
	free(scope);
}

void context_decl_arg(Context * ctx, const char * name) {
	assert(ctx->current_scope);

	int offset = ctx->current_scope->curr_arg_offset++;
	if (offset > ctx->current_scope->arg_count) abort();

	ctx->current_scope->args[offset] = name;
}


void context_decl_var(Context * ctx, const char * name) {
	assert(ctx->current_scope);

	int offset = ctx->current_scope->curr_var_offset++;
	if (offset > ctx->current_scope->var_count) abort();

	ctx->current_scope->vars[offset] = name;
}

int context_get_local_offset(Context * ctx, const char * name) {
	for (int i = 0; i < ctx->current_scope->var_count; i++) {
		if (ctx->current_scope->vars[i] && strcmp(ctx->current_scope->vars[i], name) == 0) {
			return ctx->stack_offset + i;
		}
	}
	
	for (int i = 0; i < ctx->current_scope->arg_count; i++) {
		if (strcmp(ctx->current_scope->args[i], name) == 0) {
			return ctx->stack_offset + ctx->current_scope->stack_frame_size + 1 + i;
		}
	}

	abort(); // Undeclared variable used
}

static void code_append(Context * ctx, char const * fmt, ...) {
	va_list args;
    va_start(args, fmt);

	char new_code[1024];

	char const * indent = "    ";
	int  const   indent_len = strlen(indent);

	for (int i = 0; i < ctx->indent; i++) {
		memcpy(new_code + i * indent_len, indent, indent_len);
	}
	
	vsprintf_s(new_code + ctx->indent * indent_len, sizeof(new_code) - ctx->indent * indent_len, fmt, args);
	
	va_end(args);
	
	int new_code_len = strlen(new_code);

	int new_length = ctx->code_len + new_code_len;
	if (new_length >= ctx->code_cap) {
		ctx->code_cap *= 2;
		ctx->code = realloc(ctx->code, ctx->code_cap);
	}

	memcpy(ctx->code + ctx->code_len, new_code, new_code_len + 1);
	ctx->code_len = new_length;
}

static int context_add_string_literal(Context * ctx, char const * str_lit) {
	if (ctx->string_lit_len == ctx->string_lit_cap) {
		ctx->string_lit_cap *= 2;
		ctx->string_lits = realloc(ctx->string_lits, ctx->string_lit_cap * sizeof(char const *));
	}

	int index = ctx->string_lit_len++;
	ctx->string_lits[index] = str_lit;

	return index;
}

static int  codegen_expression(Context * ctx, AST_Expression const * expr);
static void codegen_statement (Context * ctx, AST_Statement  const * stat);

static int codegen_expression_const(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_CONST);

	int val;
	switch (expr->expr_const.token.type) {
		case TOKEN_LITERAL_INT:  val = expr->expr_const.token.value_int;  break;
		case TOKEN_LITERAL_BOOL: val = expr->expr_const.token.value_char; break;

		case TOKEN_LITERAL_STRING: {
			int reg = context_reg_request(ctx);
			int lit_index = context_add_string_literal(ctx, expr->expr_const.token.value_str);

			code_append(ctx, "lea %s, [REL str_lit_%i]\n", reg_names[reg], lit_index);

			return reg;
		}

		default: abort();
	}
	
	int reg = context_reg_request(ctx);
	code_append(ctx, "mov %s, %i\n", reg_names[reg], val);

	return reg;
}

static int codegen_expression_var(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_VAR);

	char const * var_name = expr->expr_var.token.value_str;
	int offset = context_get_local_offset(ctx, var_name);

	int reg = context_reg_request(ctx);
	code_append(ctx, "mov %s, QWORD [rsp + %i * 8] ; get %s\n", reg_names[reg], offset, var_name);

	return reg;
}

// Helper function used by relational and equality operators
static void codegen_compare_branch(Context * ctx, char const * jump_instruction, char const * reg_name_left, char const * reg_name_right) {
	int label_else = context_new_label(ctx);
	int label_exit = context_new_label(ctx);

	code_append(ctx, "cmp %s, %s\n", reg_name_left, reg_name_right);
	code_append(ctx, "%s L%i\n", jump_instruction, label_else);
	code_append(ctx, "mov %s, 1\n", reg_name_left);
	code_append(ctx, "jmp L%i\n", label_exit);
	code_append(ctx, "L%i:\n", label_else);
	code_append(ctx, "mov %s, 0\n", reg_name_left);
	code_append(ctx, "L%i:\n", label_exit);
}

static int codegen_expression_op_bin(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_BIN);

	if (expr->expr_op_bin.token.type == TOKEN_ASSIGN) {
		AST_Expression const * expr_left  = expr->expr_op_bin.expr_left;
		AST_Expression const * expr_right = expr->expr_op_bin.expr_right;

		switch (expr_left->type) {
			case AST_EXPRESSION_VAR: {
				char const * var_name = expr_left->expr_var.token.value_str;
				int          var_offset = context_get_local_offset(ctx, var_name);

				if (expr_right->type == AST_EXPRESSION_CONST && expr_right->expr_const.token.type != TOKEN_LITERAL_STRING) {
					code_append(ctx, "mov QWORD [rsp + %i * 8], %i ; set %s\n", var_offset, expr_right->expr_const.token.value_int, var_name);

					return -1;
				} else {
					int reg = codegen_expression(ctx, expr_right);
					code_append(ctx, "mov QWORD [rsp + %i * 8], %s ; set %s\n", var_offset, reg_names[reg], var_name);

					return reg;
				}
			}

			case AST_EXPRESSION_OPERATOR_PRE: {
				char const * var_name = expr_left->expr_op_pre.expr->expr_var.token.value_str;
				int          var_offset = context_get_local_offset(ctx, var_name);

				int deref_reg = context_reg_request(ctx);
				code_append(ctx, "mov %s, QWORD [rsp + %i * 8]\n", reg_names[deref_reg], var_offset);

				if (expr_right->type == AST_EXPRESSION_CONST && expr_right->expr_const.token.type != TOKEN_LITERAL_STRING) {
					code_append(ctx, "mov QWORD [%s], %i ; set ptr %s\n", reg_names[deref_reg], expr_right->expr_const.token.value_int, var_name);
					context_reg_free(ctx, deref_reg);

					return -1;
				} else {
					int reg = codegen_expression(ctx, expr_right);
					code_append(ctx, "mov QWORD [%s], %s ; set ptr %s\n", reg_names[deref_reg], reg_names[reg], var_name);
					context_reg_free(ctx, deref_reg);

					return reg;
				}
			}

			default: abort(); // LHS of assignment must be variable
		}
	}

	int reg_left, reg_right;

	if (expr->expr_op_bin.expr_left->height >= expr->expr_op_bin.expr_right->height) {
		reg_left  = codegen_expression(ctx, expr->expr_op_bin.expr_left);
		reg_right = codegen_expression(ctx, expr->expr_op_bin.expr_right);
	} else {
		reg_right = codegen_expression(ctx, expr->expr_op_bin.expr_right);
		reg_left  = codegen_expression(ctx, expr->expr_op_bin.expr_left);
	}

	char const * reg_name_left  = reg_names[reg_left];
	char const * reg_name_right = reg_names[reg_right];

	switch (expr->expr_op_bin.token.type) {
		case TOKEN_OPERATOR_PLUS:  code_append(ctx, "add %s, %s\n", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_MINUS: code_append(ctx, "sub %s, %s\n", reg_name_left, reg_name_right); break;

		case TOKEN_OPERATOR_MULTIPLY: code_append(ctx, "imul %s, %s\n", reg_name_left, reg_name_right); break;
		//case TOKEN_OPERATOR_DIVIDE: {
		//	int reg_temp;

		//	if (reg_left != EAX) {
		//		reg_temp = context_reg_request(ctx);

		//		code_append(ctx, "mov %s, rax\n", reg_names[reg_temp]);
		//		code_append(ctx, "mov rax, %s\n", reg_name_left);
		//	}

		//	code_append(ctx, "cdq\nidiv %s\n", reg_name_right);

		//	if (reg_left != EAX) {
		//		code_append(ctx, "mov %s, rax\n", reg_name_left);
		//		code_append(ctx, "mov rax, %s\n", reg_names[reg_temp]);

		//		context_reg_free(ctx, reg_temp);
		//	}

		//	break;
		//}

		case TOKEN_OPERATOR_LT:    codegen_compare_branch(ctx, "jge", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_LT_EQ: codegen_compare_branch(ctx, "jg",  reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_GT:    codegen_compare_branch(ctx, "jle", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_GT_EQ: codegen_compare_branch(ctx, "jl",  reg_name_left, reg_name_right); break;

		case TOKEN_OPERATOR_EQ: codegen_compare_branch(ctx, "jne", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_NE: codegen_compare_branch(ctx, "je",  reg_name_left, reg_name_right); break;

		default: abort();
	}

	context_reg_free(ctx, reg_right);

	return reg_left;
}

static int codegen_expression_op_pre(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_PRE);

	Token_Type operator = expr->expr_op_pre.token.type;

	// Check if this is a pointer operator
	if (operator == TOKEN_OPERATOR_BITWISE_AND || operator == TOKEN_OPERATOR_MULTIPLY) {
		AST_Expression * operand = expr->expr_op_pre.expr;
		if (operand->type != AST_EXPRESSION_VAR) abort(); // Pointer operators only work on identifiers

		const char * var_name = operand->expr_var.token.value_str;
		int          var_offset = context_get_local_offset(ctx, var_name);

		int result_reg = context_reg_request(ctx);

		if (operator == TOKEN_OPERATOR_BITWISE_AND) {
			code_append(ctx, "lea %s, QWORD [RSP + %i * 8] ; addrof %s\n", reg_names[result_reg], var_offset, var_name);
		} else {
			code_append(ctx, "mov %s, QWORD [RSP + %i * 8] ; deref %s\n", reg_names[result_reg], var_offset, var_name);
			code_append(ctx, "mov %s, QWORD [%s]\n",                      reg_names[result_reg], reg_names[result_reg]);
		}

		return result_reg;
	}

	int reg = codegen_expression(ctx, expr->expr_op_pre.expr);

	switch (operator) {
		case TOKEN_OPERATOR_PLUS: break; // Do nothing
		case TOKEN_OPERATOR_MINUS: code_append(ctx, "neg %s\n", reg_names[reg]); break;


		default: abort();
	}

	return reg;
}

static int codegen_expression_call_func(Context * ctx, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_CALL_FUNC);
	
	int            arg_count = 0;
	AST_Call_Arg * arg = expr->expr_call.args;

	while (arg) {
		arg_count++;
		arg = arg->next;
	}

	arg = expr->expr_call.args;

	int stack_offset;
	if (arg_count < 4) {
		stack_offset = 4;
		code_append(ctx, "sub rsp, 32 ; shadow space\n");
	} else if (arg_count & 1) {
		stack_offset = arg_count + 1;
		code_append(ctx, "sub rsp, 32 + %i * 8 + 8 ; shadow space + spill arguments + alignment\n", arg_count - 4);
	} else {
		stack_offset = arg_count;
		code_append(ctx, "sub rsp, 32 + %i * 8 ; shadow space + spill arguments\n", arg_count - 4);
	}

	ctx->stack_offset += stack_offset;

	int arg_index = 0;
	while (arg) {
		int arg_reg = codegen_expression(ctx, arg->expr);
		if (arg_index < 4) {
			code_append(ctx, "mov %s, %s ; arg %i\n", function_call_reg_names[arg_index], reg_names[arg_reg], arg_index);
		} else {
			code_append(ctx, "mov QWORD [RSP + %i * 8], %s ; arg %i\n", arg_index, reg_names[arg_reg], arg_index);
		}
		context_reg_free(ctx, arg_reg);

		arg_index++;
		arg = arg->next;
	}

	int reg = context_reg_request(ctx);
	int tmp;

	if (reg != RAX) {
		tmp = context_reg_request(ctx);
		code_append(ctx, "mov %s, rax ; save rax\n", reg_names[tmp]);
	}

	code_append(ctx, "call %s\n", expr->expr_call.function);
	
	ctx->stack_offset -= stack_offset;
	code_append(ctx, "add rsp, %i * 8\n", stack_offset);

	if (reg != RAX) {
		code_append(ctx, "mov %s, rax\n", reg_names[reg]);
		code_append(ctx, "mov rax, %s ; restore rax\n", reg_names[tmp]);

		context_reg_free(ctx, tmp);
	}

	return reg;
}

static int codegen_expression(Context * ctx, AST_Expression const * expr) {
	switch (expr->type) {
		case AST_EXPRESSION_CONST: return codegen_expression_const(ctx, expr);
		case AST_EXPRESSION_VAR:   return codegen_expression_var  (ctx, expr);

		//case AST_EXPRESSION_ASSIGN:
		case AST_EXPRESSION_OPERATOR_BIN:  return codegen_expression_op_bin (ctx, expr);
		case AST_EXPRESSION_OPERATOR_PRE:  return codegen_expression_op_pre (ctx, expr);
		//case AST_EXPRESSION_OPERATOR_POST: return codegen_expression_op_post(ctx, expr);

		case AST_EXPRESSION_CALL_FUNC: return codegen_expression_call_func(ctx, expr);

		default: abort();
	}
}

static void codegen_statement_noop(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_NOOP);

	code_append(ctx, "nop\n");
}

static void codegen_statement_statements(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENTS);

	if (stat->stat_stats.head) codegen_statement(ctx, stat->stat_stats.head);
	if (stat->stat_stats.cons) codegen_statement(ctx, stat->stat_stats.cons);
}

static void codegen_statement_expression(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXPR);

	int reg = codegen_expression(ctx, stat->stat_expr.expr);
	context_reg_free(ctx, reg);
}

static void codegen_statement_decl_var(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DECL_VAR);

	context_decl_var(ctx, stat->stat_decl_var.name);
}

static int count_vars_in_function(AST_Statement const * stat) {
	if (stat->type != AST_STATEMENTS) return 0;

	int count = 0;

	if (stat->stat_stats.head) {
		AST_Statement * head = stat->stat_stats.head;
		if (head->type == AST_STATEMENT_DECL_VAR) {
			count = 1;
		} else if (head->type == AST_STATEMENT_IF) {
			count += count_vars_in_function(head->stat_if.case_true);

			if (head->stat_if.case_false) {
				count += count_vars_in_function(head->stat_if.case_false);
			}
		} else if (head->type == AST_STATEMENT_WHILE) {
			count += count_vars_in_function(head->stat_while.body);
		}
	}
	if (stat->stat_stats.cons) {
		count += count_vars_in_function(stat->stat_stats.cons);
	}

	return count;
}

static void codegen_statement_decl_func(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DECL_FUNC);

	code_append(ctx, "%s:\n", stat->stat_decl_func.name);
	ctx->indent++;

	// Count arguments
	int            arg_count = 0;
	AST_Decl_Arg * arg = stat->stat_decl_func.args;

	while (arg) {
		arg_count++;
		arg = arg->next;
	}
	
	// Count variables
	int var_count = count_vars_in_function(stat->stat_decl_func.body);

	// Create new scope
	context_scope_push(ctx, arg_count, var_count);

	// Push arguments on stack
	int arg_offset = 0;
	arg = stat->stat_decl_func.args;

	while (arg) {
		context_decl_arg(ctx, arg->name);

		if (arg_offset < 4) {
			code_append(ctx, "mov QWORD [rsp + %i * 8], %s\n", arg_offset + 1, function_call_reg_names[arg_offset]);
		}

		arg_offset++;
		arg = arg->next;
	}

	// Reserve space on stack for local variables
	int stack_frame_size;
	if ((var_count & 1) == 0) {
		stack_frame_size = var_count + 1;
		code_append(ctx, "sub rsp, %i * 8 + 8; %i vars + alignment\n", var_count, var_count);
	} else {
		stack_frame_size = var_count;
		code_append(ctx, "sub rsp, %i * 8; %i vars\n", var_count, var_count);
	}

	ctx->current_scope->stack_frame_size = stack_frame_size;

	// Function body
	codegen_statement(ctx, stat->stat_decl_func.body);

	code_append(ctx, "; Default return\n");
	code_append(ctx, "add rsp, %i\n", stack_frame_size * 8);
	code_append(ctx, "xor rax, rax\n");
	code_append(ctx, "ret\n");
	code_append(ctx, "\n");

	context_scope_pop(ctx);
	
	ctx->indent--;
}

static void codegen_statement_extern(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXTERN);

	code_append(ctx, "EXTERN %s\n\n", stat->stat_extern.name);
}

static void codegen_statement_if(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_IF);

	int reg = codegen_expression(ctx, stat->stat_if.condition);
	context_reg_free(ctx, reg);

	int label = context_new_label(ctx);
	code_append(ctx, "cmp %s, 0\n", reg_names[reg]);

	if (stat->stat_if.case_false == NULL) {
		code_append(ctx, "je L_exit%i\n", label);
		
		ctx->indent++;
		codegen_statement(ctx, stat->stat_if.case_true);
		ctx->indent--;
	} else {
		code_append(ctx, "je L_else%i\n", label);

		ctx->indent++;
		codegen_statement(ctx, stat->stat_if.case_true);
		ctx->indent--;

		code_append(ctx, "jmp L_exit%i\n", label);
		code_append(ctx, "L_else%i:\n",    label);
		
		ctx->indent++;
		codegen_statement(ctx, stat->stat_if.case_false);
		ctx->indent--;
	}
	
	code_append(ctx, "L_exit%i:\n", label);
}

static void codegen_statement_while(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_WHILE);

	int label = context_new_label(ctx);
	code_append(ctx, "L_loop%i:\n", label);
	
	int reg = codegen_expression(ctx, stat->stat_while.condition);
	context_reg_free(ctx, reg);

	code_append(ctx, "cmp %s, 0\n", reg_names[reg]);
	code_append(ctx, "je L_exit%i\n", label);

	ctx->current_loop_label = label;
	ctx->indent++;
	codegen_statement(ctx, stat->stat_while.body);
	ctx->indent--;
	ctx->current_loop_label = -1;

	code_append(ctx, "jmp L_loop%i\n", label);
	code_append(ctx, "L_exit%i:\n", label);
}

static void codegen_statement_break(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_BREAK);

	if (ctx->current_loop_label == -1) abort();

	code_append(ctx, "jmp L_exit%i\n", ctx->current_loop_label);
}

static void codegen_statement_continue(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_CONTINUE);
	
	if (ctx->current_loop_label == -1) abort();
	
	code_append(ctx, "jmp L_loop%i\n", ctx->current_loop_label);
}

static void codegen_statement_return(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_RETURN);

	if (stat->stat_return.expr) {
		int reg_return = codegen_expression(ctx, stat->stat_return.expr);

		if (reg_return != RAX) {
			code_append(ctx, "mov rax, %s ; Return via rax\n", reg_names[reg_return]);
		}

		context_reg_free(ctx, reg_return);
	} else {
		code_append(ctx, "mov rax, 0\n");
	}
	
	code_append(ctx, "add rsp, %i\n", ctx->current_scope->stack_frame_size * 8);
	code_append(ctx, "ret\n");
}

static void codegen_statement(Context * ctx, AST_Statement const * stat) {
	switch (stat->type) {
		case AST_STATEMENT_NOOP: codegen_statement_noop      (ctx, stat); break;
		case AST_STATEMENTS:     codegen_statement_statements(ctx, stat); break;

		case AST_STATEMENT_EXPR: codegen_statement_expression(ctx, stat); break;

		case AST_STATEMENT_DECL_VAR:  codegen_statement_decl_var (ctx, stat); break;
		case AST_STATEMENT_DECL_FUNC: codegen_statement_decl_func(ctx, stat); break;
		case AST_STATEMENT_EXTERN:    codegen_statement_extern   (ctx, stat); break;

		case AST_STATEMENT_IF:    codegen_statement_if   (ctx, stat); break;
		case AST_STATEMENT_WHILE: codegen_statement_while(ctx, stat); break;

		case AST_STATEMENT_BREAK:    codegen_statement_break   (ctx, stat); break;
		case AST_STATEMENT_CONTINUE: codegen_statement_continue(ctx, stat); break;
		case AST_STATEMENT_RETURN:   codegen_statement_return  (ctx, stat); break;

		default: abort();
	}
}

char const * codegen_program(AST_Statement const * program) {
	Context ctx;
	context_init(&ctx);

	char const asm_init[] =
		"GLOBAL main\n\n"

		"SECTION .code\n";

	code_append(&ctx, asm_init);

	codegen_statement(&ctx, program);

	code_append(&ctx, "SECTION .data\n");

	for (int i = 0; i < ctx.string_lit_len; i++) {
		code_append(&ctx, "str_lit_%i db \"%s\", 0\n", i, ctx.string_lits[i]);
	}

	return ctx.code;
}

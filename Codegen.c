#include "Godegen.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

static char const * reg_names_call   [] = { "rcx", "rdx", "r8", "r9" }; 
static char const * reg_names_scratch[] = { "rbx", "r10", "r11", "r12", "r13", "r14", "r15" };

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

typedef enum Context_Flags {
	CTX_FLAG_VAR_BY_ADDRESS = 1
} Context_Flags;

typedef struct Context {
	int scratch_reg_mask;

	int indent;

	int label;
	int current_loop_label;

	unsigned flags;

	Scope * current_scope;
	int stack_offset;

	char * code;
	int    code_len;
	int    code_cap;

	char const ** data_seg_vals;
	int           data_seg_len;
	int           data_seg_cap;
} Context;

static void context_init(Context * ctx) {
	ctx->scratch_reg_mask = 0;

	ctx->indent = 0;

	ctx->label = 0;
	ctx->current_loop_label = -1;

	ctx->flags = 0;

	ctx->current_scope = NULL;
	ctx->stack_offset = 0;

	ctx->code_len = 0;
	ctx->code_cap = 512;
	ctx->code     = malloc(ctx->code_cap);

	ctx->data_seg_len  = 0;
	ctx->data_seg_cap  = 16;
	ctx->data_seg_vals = malloc(ctx->data_seg_cap * sizeof(char const *));
}

static int context_reg_request(Context * ctx) {
	int const reg_count = sizeof(reg_names_scratch) / sizeof(char const *);

	for (int i = 0; i < reg_count; i++) {
		if ((ctx->scratch_reg_mask & (1 << i)) == 0) {
			ctx->scratch_reg_mask |= (1 << i);

			return i;
		}
	}

	abort(); // No registers available!
}

static void context_reg_free(Context * ctx, int reg) {
	if (reg == -1) return;

	assert(ctx->scratch_reg_mask & (1 << reg));

	ctx->scratch_reg_mask &= ~(1 << reg);
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

static Scope * context_scope_push(Context * ctx, int arg_count, int var_count) {
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

static void context_scope_pop(Context * ctx) {
	Scope * scope = ctx->current_scope;
	ctx->current_scope = scope->prev;

	free(scope->args);
	free(scope->vars);
	free(scope);
}

static void context_decl_arg(Context * ctx, const char * name) {
	assert(ctx->current_scope);

	int offset = ctx->current_scope->curr_arg_offset++;
	if (offset > ctx->current_scope->arg_count) abort();

	ctx->current_scope->args[offset] = name;
}


static void context_decl_var(Context * ctx, const char * name) {
	assert(ctx->current_scope);

	int offset = ctx->current_scope->curr_var_offset++;
	if (offset > ctx->current_scope->var_count) abort();

	ctx->current_scope->vars[offset] = name;
}

static void context_get_variable(Context * ctx, const char * name, char * address, int address_size) {
	for (int i = 0; i < ctx->current_scope->var_count; i++) {
		if (ctx->current_scope->vars[i] && strcmp(ctx->current_scope->vars[i], name) == 0) {
			sprintf_s(address, address_size, "rsp + %i * 8", ctx->stack_offset + i);

			return;
		}
	}
	
	for (int i = 0; i < ctx->current_scope->arg_count; i++) {
		if (strcmp(ctx->current_scope->args[i], name) == 0) {
			sprintf_s(address, address_size, "rsp + %i * 8", ctx->stack_offset + ctx->current_scope->stack_frame_size + 1 + i);

			return;
		}
	}

	sprintf_s(address, address_size, "REL %s", name); // Globals by name
}

static void context_emit_code(Context * ctx, char const * fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char new_code[1024];

	// Add indentation based on current Context
	char const * indent = "    ";
	int  const   indent_len = strlen(indent);

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

static int context_add_global(Context * ctx, char const * var_name, int value) {
	if (ctx->data_seg_len == ctx->data_seg_cap) {
		ctx->data_seg_cap *= 2;
		ctx->data_seg_vals = realloc(ctx->data_seg_vals, ctx->data_seg_cap * sizeof(char const *));
	}

	int var_name_len = strlen(var_name);

	int    global_size = var_name_len + 5 + 32;
	char * global = malloc(global_size);

	sprintf_s(global, global_size, "%s dq %i", var_name, value);

	int index = ctx->data_seg_len++;
	ctx->data_seg_vals[index] = global;

	return index;
}

static int context_add_string_literal(Context * ctx, char const * str_lit) {
	if (ctx->data_seg_len == ctx->data_seg_cap) {
		ctx->data_seg_cap *= 2;
		ctx->data_seg_vals = realloc(ctx->data_seg_vals, ctx->data_seg_cap * sizeof(char const *));
	}

	int str_lit_len = strlen(str_lit);

	int    str_lit_cpy_size = str_lit_len * 9 + 1;
	char * str_lit_cpy      = malloc(str_lit_cpy_size);

	int idx = sprintf_s(str_lit_cpy, str_lit_cpy_size, "str_lit_%i db ", ctx->data_seg_len);

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

	strcat_s(str_lit_cpy, str_lit_cpy_size - idx, ", 0");

	int index = ctx->data_seg_len++;
	ctx->data_seg_vals[index] = str_lit_cpy;

	return index;
}

static int  codegen_expression(Context * ctx, AST_Expression const * expr);
static void codegen_statement (Context * ctx, AST_Statement  const * stat);

static int codegen_expression_const(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_CONST);

	int reg = context_reg_request(ctx);

	Token_Type literal_type = expr->expr_const.token.type;

	if (literal_type == TOKEN_LITERAL_STRING) {
		int str_lit_index = context_add_string_literal(ctx, expr->expr_const.token.value_str);

		context_emit_code(ctx, "lea %s, [REL str_lit_%i]\n", reg_names_scratch[reg], str_lit_index);
	} else {
		int val;
		
		if (literal_type == TOKEN_LITERAL_INT) {
			val = expr->expr_const.token.value_int;
		} else if (literal_type == TOKEN_LITERAL_BOOL) {
			val = expr->expr_const.token.value_char;
		} else {
			abort();
		}

		context_emit_code(ctx, "mov %s, %i\n", reg_names_scratch[reg], val);
	}
	
	return reg;
}

static int codegen_expression_var(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_VAR);

	char const * var_name = expr->expr_var.name;
	char         var_address[32];
	context_get_variable(ctx, var_name, var_address, sizeof(var_address));

	int reg = context_reg_request(ctx);

	if (ctx->flags & CTX_FLAG_VAR_BY_ADDRESS) {
		context_emit_code(ctx, "lea %s, QWORD [%s] ; addr of %s\n", reg_names_scratch[reg], var_address, var_name);
	} else {
		context_emit_code(ctx, "mov %s, QWORD [%s] ; get %s\n",     reg_names_scratch[reg], var_address, var_name);
	}

	return reg;
}

// Helper function used by relational and equality operators
static void codegen_compare_branch(Context * ctx, char const * jump_instruction, char const * reg_name_left, char const * reg_name_right) {
	int label_else = context_new_label(ctx);
	int label_exit = context_new_label(ctx);

	context_emit_code(ctx, "cmp %s, %s\n", reg_name_left, reg_name_right);
	context_emit_code(ctx, "%s L%i\n", jump_instruction, label_else);
	context_emit_code(ctx, "mov %s, 1\n", reg_name_left);
	context_emit_code(ctx, "jmp L%i\n", label_exit);
	context_emit_code(ctx, "L%i:\n",    label_else);
	context_emit_code(ctx, "mov %s, 0\n", reg_name_left);
	context_emit_code(ctx, "L%i:\n", label_exit);
}

static int codegen_expression_op_bin(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_BIN);

	Token_Type operator = expr->expr_op_bin.token.type;
		
	AST_Expression const * expr_left  = expr->expr_op_bin.expr_left;
	AST_Expression const * expr_right = expr->expr_op_bin.expr_right;

	// Assignment operator is handled separately
	if (operator == TOKEN_ASSIGN) {
		// Evaluate rhs first
		int reg_right = codegen_expression(ctx, expr_right);

		// Evaluate lhs by address
		context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);
		int reg_left = codegen_expression(ctx, expr_left);
		context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);

		context_emit_code(ctx, "mov QWORD [%s], %s\n", reg_names_scratch[reg_left], reg_names_scratch[reg_right]);

		context_reg_free(ctx, reg_right);

		return reg_left;
	}

	// Logical And and Or ('&&' and '||') have short circuit evaluation
	//if (expr->expr_op_bin.token.type == TOKEN_OPERATOR_LOGICAL_AND ||
	//	expr->expr_op_bin.token.type == TOKEN_OPERATOR_LOGICAL_OR
	//) {

	//}

	int reg_left, reg_right;

	// Traverse tallest subtree first
	if (expr->expr_op_bin.expr_left->height >= expr->expr_op_bin.expr_right->height) {
		reg_left  = codegen_expression(ctx, expr_left);
		reg_right = codegen_expression(ctx, expr_right);
	} else {
		reg_right = codegen_expression(ctx, expr_right);
		reg_left  = codegen_expression(ctx, expr_left);
	}

	char const * reg_name_left  = reg_names_scratch[reg_left];
	char const * reg_name_right = reg_names_scratch[reg_right];

	// Emit correct instructions based on operator type
	switch (operator) {
		case TOKEN_OPERATOR_PLUS:  context_emit_code(ctx, "add %s, %s\n", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_MINUS: context_emit_code(ctx, "sub %s, %s\n", reg_name_left, reg_name_right); break;

		case TOKEN_OPERATOR_MULTIPLY: context_emit_code(ctx, "imul %s, %s\n", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_DIVIDE: {
			context_emit_code(ctx, "mov rax, %s\n", reg_name_left);
			context_emit_code(ctx, "cdq\n");
			context_emit_code(ctx, "idiv %s\n",     reg_name_right);
			context_emit_code(ctx, "mov %s, rax\n", reg_name_left);

			break;
		}
		case TOKEN_OPERATOR_MODULO: {
			context_emit_code(ctx, "mov rax, %s\n", reg_name_left);
			context_emit_code(ctx, "cdq\n");
			context_emit_code(ctx, "idiv %s\n",     reg_name_right);
			context_emit_code(ctx, "mov %s, rdx\n", reg_name_left); // RDX contains remainder after division

			break;
		}

		case TOKEN_OPERATOR_LT:    codegen_compare_branch(ctx, "jge", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_LT_EQ: codegen_compare_branch(ctx, "jg",  reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_GT:    codegen_compare_branch(ctx, "jle", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_GT_EQ: codegen_compare_branch(ctx, "jl",  reg_name_left, reg_name_right); break;

		case TOKEN_OPERATOR_EQ: codegen_compare_branch(ctx, "jne", reg_name_left, reg_name_right); break;
		case TOKEN_OPERATOR_NE: codegen_compare_branch(ctx, "je",  reg_name_left, reg_name_right); break;

		case TOKEN_OPERATOR_LOGICAL_AND: {
			int label = context_new_label(ctx);

			context_emit_code(ctx, "test %s, %s\n", reg_name_left, reg_name_left);
			context_emit_code(ctx, "je L_land_false_%i\n", label);
			context_emit_code(ctx, "test %s, %s\n", reg_name_right, reg_name_right);
			context_emit_code(ctx, "je L_land_false_%i\n", label);
			context_emit_code(ctx, "mov %s, 1\n",   reg_name_left);
			context_emit_code(ctx, "jmp L_land_exit_%i\n", label);
			context_emit_code(ctx, "L_land_false_%i:\n", label);
			context_emit_code(ctx, "mov %s, 0\n",   reg_name_left);
			context_emit_code(ctx, "L_land_exit_%i:\n",  label);

			break;
		}

		case TOKEN_OPERATOR_LOGICAL_OR: {
			int label = context_new_label(ctx);

			context_emit_code(ctx, "test %s, %s\n", reg_name_left, reg_name_left);
			context_emit_code(ctx, "jne L_lor_true_%i\n", label);
			context_emit_code(ctx, "test %s, %s\n", reg_name_right, reg_name_right);
			context_emit_code(ctx, "jne L_lor_true_%i\n", label);
			context_emit_code(ctx, "mov %s, 0\n",   reg_name_left);
			context_emit_code(ctx, "jmp L_lor_exit_%i\n", label);
			context_emit_code(ctx, "L_lor_true_%i:\n", label);
			context_emit_code(ctx, "mov %s, 1\n",   reg_name_left);
			context_emit_code(ctx, "L_lor_exit_%i:\n",  label);

			break;
		}

		default: abort();
	}

	context_reg_free(ctx, reg_right);

	return reg_left;
}

static int codegen_expression_op_pre(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_PRE);

	AST_Expression * operand  = expr->expr_op_pre.expr;
	Token_Type       operator = expr->expr_op_pre.token.type;

	// Check if this is a pointer operator
	if (operator == TOKEN_OPERATOR_BITWISE_AND) {
		if (operand->type != AST_EXPRESSION_VAR) abort(); // Can only take address of variable
		
		const char * var_name   = operand->expr_var.name;
		char         var_address[32];
		context_get_variable(ctx, var_name, var_address, sizeof(var_address));

		int reg = context_reg_request(ctx);

		context_emit_code(ctx, "lea %s, QWORD [%s] ; addrof %s\n", reg_names_scratch[reg], var_address, var_name);

		return reg;
	} else if (operator == TOKEN_OPERATOR_MULTIPLY) {
		bool var_by_address = ctx->flags & CTX_FLAG_VAR_BY_ADDRESS;

		context_flag_unset(ctx, CTX_FLAG_VAR_BY_ADDRESS);
		int reg = codegen_expression(ctx, operand);

		if (var_by_address) {
			context_flag_set(ctx, CTX_FLAG_VAR_BY_ADDRESS);
		} else {
			context_emit_code(ctx, "mov %s, QWORD [%s]\n", reg_names_scratch[reg], reg_names_scratch[reg]);
		}

		return reg;
	}

	int          reg = codegen_expression(ctx, operand);
	char const * reg_name = reg_names_scratch[reg];

	switch (operator) {
		case TOKEN_OPERATOR_PLUS: break; // Do nothing
		case TOKEN_OPERATOR_MINUS: context_emit_code(ctx, "neg %s\n", reg_name); break;

		case TOKEN_OPERATOR_LOGICAL_NOT: {
			int label = context_new_label(ctx);

			context_emit_code(ctx, "test %s, %s\n", reg_name, reg_name);
			context_emit_code(ctx, "jne L_lnot_false_%i\n", label);
			context_emit_code(ctx, "mov %s, 1\n",   reg_name);
			context_emit_code(ctx, "jmp L_lnot_exit_%i\n", label);
			context_emit_code(ctx, "L_lnot_false_%i:\n", label);
			context_emit_code(ctx, "mov %s, 0\n",   reg_name);
			context_emit_code(ctx, "L_lnot_exit_%i:\n",  label);

			break;

			break;
		}

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
		context_emit_code(ctx, "sub rsp, 32 ; shadow space\n");
	} else if (arg_count & 1) {
		stack_offset = arg_count + 1;
		context_emit_code(ctx, "sub rsp, 32 + %i * 8 + 8 ; shadow space + spill arguments + alignment\n", arg_count - 4);
	} else {
		stack_offset = arg_count;
		context_emit_code(ctx, "sub rsp, 32 + %i * 8 ; shadow space + spill arguments\n", arg_count - 4);
	}

	ctx->stack_offset += stack_offset;

	int arg_index = 0;
	while (arg) {
		int arg_reg = codegen_expression(ctx, arg->expr);
		if (arg_index < 4) {
			context_emit_code(ctx, "mov %s, %s ; arg %i\n", reg_names_call[arg_index], reg_names_scratch[arg_reg], arg_index);
		} else {
			context_emit_code(ctx, "mov QWORD [RSP + %i * 8], %s ; arg %i\n", arg_index, reg_names_scratch[arg_reg], arg_index);
		}
		context_reg_free(ctx, arg_reg);

		arg_index++;
		arg = arg->next;
	}

	context_emit_code(ctx, "call %s\n", expr->expr_call.function);
	
	ctx->stack_offset -= stack_offset;
	context_emit_code(ctx, "add rsp, %i * 8\n", stack_offset);
	
	int reg = context_reg_request(ctx);
	context_emit_code(ctx, "mov %s, rax ; get return value\n", reg_names_scratch[reg]);

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

	context_emit_code(ctx, "nop\n");
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

	char const * var_name = stat->stat_decl_var.name;

	if (ctx->current_scope == NULL) {
		if (stat->stat_decl_var.value == NULL) {
			context_add_global(ctx, var_name, 0);
		} else {
			if (stat->stat_decl_var.value->type != AST_EXPRESSION_CONST) abort(); // Globals can only be initialized to constant values

			if (stat->stat_decl_var.value->expr_const.token.type == TOKEN_LITERAL_STRING) abort(); // TODO

			context_add_global(ctx, var_name, stat->stat_decl_var.value->expr_const.token.value_int);
		}
	} else {
		context_decl_var(ctx, var_name);

		char var_address[32];
		context_get_variable(ctx, var_name, var_address, sizeof(var_address));

		if (stat->stat_decl_var.value) {
			int reg = codegen_expression(ctx, stat->stat_decl_var.value);

			context_emit_code(ctx, "mov QWORD [%s], %s; initialize %s\n", var_address, reg_names_scratch[reg], var_name);

			context_reg_free(ctx, reg);
		} else {
			context_emit_code(ctx, "mov QWORD [%s], 0; zero initialize %s\n", var_address, var_name);
		}
	}
}

static int count_vars_in_function(AST_Statement const * stat) {
	int count = 0;

	if (stat == NULL) return count;

	if (stat->type != AST_STATEMENTS) {
		if (stat->type == AST_STATEMENT_DECL_VAR) {
			count = 1;
		} else if (stat->type == AST_STATEMENT_IF) {
			count += count_vars_in_function(stat->stat_if.case_true);

			if (stat->stat_if.case_false) {
				count += count_vars_in_function(stat->stat_if.case_false);
			}
		} else if (stat->type == AST_STATEMENT_WHILE) {
			count += count_vars_in_function(stat->stat_while.body);
		}

		return count;
	}

	if (stat->stat_stats.head) count += count_vars_in_function(stat->stat_stats.head);
	if (stat->stat_stats.cons) count += count_vars_in_function(stat->stat_stats.cons);

	return count;
}

static void codegen_statement_decl_func(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DECL_FUNC);

	context_emit_code(ctx, "%s:\n", stat->stat_decl_func.name);
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
			context_emit_code(ctx, "mov QWORD [rsp + %i * 8], %s\n", arg_offset + 1, reg_names_call[arg_offset]);
		}

		arg_offset++;
		arg = arg->next;
	}

	// Reserve space on stack for local variables
	int stack_frame_size;
	if ((var_count & 1) == 0) {
		stack_frame_size = var_count + 1;
		context_emit_code(ctx, "sub rsp, %i * 8 + 8; %i vars + alignment\n", var_count, var_count);
	} else {
		stack_frame_size = var_count;
		context_emit_code(ctx, "sub rsp, %i * 8; %i vars\n", var_count, var_count);
	}

	ctx->current_scope->stack_frame_size = stack_frame_size;

	// Function body
	codegen_statement(ctx, stat->stat_decl_func.body);

	context_emit_code(ctx, "; Default return\n");
	context_emit_code(ctx, "add rsp, %i\n", stack_frame_size * 8);
	context_emit_code(ctx, "xor rax, rax\n");
	context_emit_code(ctx, "ret\n");
	context_emit_code(ctx, "\n");

	context_scope_pop(ctx);
	
	ctx->indent--;
}

static void codegen_statement_extern(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXTERN);

	context_emit_code(ctx, "EXTERN %s\n\n", stat->stat_extern.name);
}

static void codegen_statement_if(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_IF);

	int reg = codegen_expression(ctx, stat->stat_if.condition);
	context_reg_free(ctx, reg);

	int label = context_new_label(ctx);
	context_emit_code(ctx, "cmp %s, 0\n", reg_names_scratch[reg]);

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
	
	int reg = codegen_expression(ctx, stat->stat_while.condition);
	context_reg_free(ctx, reg);

	context_emit_code(ctx, "cmp %s, 0\n", reg_names_scratch[reg]);
	context_emit_code(ctx, "je L_exit%i\n", label);

	ctx->current_loop_label = label;
	ctx->indent++;
	codegen_statement(ctx, stat->stat_while.body);
	ctx->indent--;
	ctx->current_loop_label = -1;

	context_emit_code(ctx, "jmp L_loop%i\n", label);
	context_emit_code(ctx, "L_exit%i:\n", label);
}

static void codegen_statement_break(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_BREAK);

	if (ctx->current_loop_label == -1) abort();

	context_emit_code(ctx, "jmp L_exit%i\n", ctx->current_loop_label);
}

static void codegen_statement_continue(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_CONTINUE);
	
	if (ctx->current_loop_label == -1) abort();
	
	context_emit_code(ctx, "jmp L_loop%i\n", ctx->current_loop_label);
}

static void codegen_statement_return(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_RETURN);

	if (stat->stat_return.expr) {
		int reg_return = codegen_expression(ctx, stat->stat_return.expr);

		context_emit_code(ctx, "mov rax, %s ; return via rax\n", reg_names_scratch[reg_return]);

		context_reg_free(ctx, reg_return);
	} else {
		context_emit_code(ctx, "mov rax, 0\n");
	}
	
	context_emit_code(ctx, "add rsp, %i\n", ctx->current_scope->stack_frame_size * 8);
	context_emit_code(ctx, "ret\n");
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

#include "Godegen.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3
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
	[EAX] = "eax",
	[EBX] = "ebx",
	[ECX] = "ecx",
	[EDX] = "edx",
	[R8 ] = "r8d",
	[R9 ] = "r9d",
	[R10] = "r10d",
	[R11] = "r11d",
	[R12] = "r12d",
	[R13] = "r13d",
	[R14] = "r14d",
	[R15] = "r15d",
};

typedef struct Context {
	bool regs_occupied[12];

	int indent;

	int label;
	int current_loop_label;

	char const ** variables;
	char const ** functions;

	char * code;
	int    code_len;
	int    code_cap;
} Context;

void context_init(Context * context) {
	memset(context->regs_occupied, 0, sizeof(context->regs_occupied));

	context->indent = 0;

	context->label = 0;
	context->current_loop_label = -1;

	context->variables = malloc(sizeof(char const *) * 64);
	context->functions = malloc(sizeof(char const *) * 32);

	const int initial_capacity = 512;

	context->code = malloc(initial_capacity);
	context->code_len = 0;
	context->code_cap = initial_capacity;
}

int context_reg_request(Context * context) {
	for (int i = 0; i < REGISTER_COUNT; i++) {
		if (!context->regs_occupied[i]) {
			context->regs_occupied[i] = true;

			return i;
		}
	}

	abort(); // No registers available!
}

void context_reg_free(Context * context, int reg) {
	if (reg == -1) return;

	assert(context->regs_occupied[reg]);

	context->regs_occupied[reg] = false;
}

int context_new_label(Context * ctx) {
	return ctx->label++;
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

static int  codegen_expression(Context * ctx, AST_Expression const * expr);
static void codegen_statement (Context * ctx, AST_Statement  const * stat);

static int codegen_expression_const(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_CONST);

	int val;
	switch (expr->expr_const.token.type) {
		case TOKEN_LITERAL_INT:  val = expr->expr_const.token.value_int;  break;
		case TOKEN_LITERAL_BOOL: val = expr->expr_const.token.value_char; break;
		//case TOKEN_LITERAL_STRING: code_append(ctx, "mov %s, %i\n", reg_names[reg], expr->expr_const.token.value_int); break;

		default: abort();
	}
	
	int reg = context_reg_request(ctx);
	code_append(ctx, "mov %s, %i\n", reg_names[reg], val);

	return reg;
}

static int codegen_expression_var(Context * ctx, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_VAR);

	int reg = context_reg_request(ctx);
	code_append(ctx, "mov %s, DWORD [REL %s]\n", reg_names[reg], expr->expr_var.token.value_str);

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
		char const * var = expr->expr_op_bin.expr_left->expr_var.token.value_str;

		AST_Expression const * expr_right = expr->expr_op_bin.expr_right;

		if (expr_right->type == AST_EXPRESSION_CONST) {
			code_append(ctx, "mov DWORD [REL %s], %i\n", var, expr_right->expr_const.token.value_int);

			return -1;
		} else {
			int reg = codegen_expression(ctx, expr_right);
			code_append(ctx, "mov DWORD [REL %s], %s\n", var, reg_names[reg]);

			return reg;
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

		//		code_append(ctx, "mov %s, eax\n", reg_names[reg_temp]);
		//		code_append(ctx, "mov eax, %s\n", reg_name_left);
		//	}

		//	code_append(ctx, "cdq\nidiv %s\n", reg_name_right);

		//	if (reg_left != EAX) {
		//		code_append(ctx, "mov %s, eax\n", reg_name_left);
		//		code_append(ctx, "mov eax, %s\n", reg_names[reg_temp]);

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

static int codegen_expression_call_func(Context * ctx, AST_Expression * expr) {
	assert(expr->type == AST_EXPRESSION_CALL_FUNC);

	int reg = context_reg_request(ctx);
	int tmp;

	if (reg != EAX) {
		tmp = context_reg_request(ctx);
		code_append(ctx, "mov %s, eax ; save eax\n", reg_names[tmp]);
	}

	code_append(ctx, "call %s\n", expr->expr_call.function);

	if (reg != EAX) {
		code_append(ctx, "mov %s, eax\n", reg_names[reg]);
		code_append(ctx, "mov eax, %s ; restore eax\n", reg_names[tmp]);

		context_reg_free(ctx, tmp);
	}

	return reg;
}

static int codegen_expression(Context * ctx, AST_Expression const * expr) {
	switch (expr->type) {
		case AST_EXPRESSION_CONST: return codegen_expression_const(ctx, expr);
		case AST_EXPRESSION_VAR:   return codegen_expression_var  (ctx, expr);

		//case AST_EXPRESSION_ASSIGN:
		case AST_EXPRESSION_OPERATOR_BIN: return codegen_expression_op_bin(ctx, expr);
		//case AST_EXPRESSION_OPERATOR_PRE:
		//case AST_EXPRESSION_OPERATOR_POST:
		case AST_EXPRESSION_CALL_FUNC: return codegen_expression_call_func(ctx, expr);
	}
}

static void codegen_statement_noop(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_NOOP);

	code_append(ctx, "nop\n");
}

static void codegen_statement_statements(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENTS);

	if (stat->stat_statements.head) codegen_statement(ctx, stat->stat_statements.head);
	if (stat->stat_statements.cons) codegen_statement(ctx, stat->stat_statements.cons);
}

static void codegen_statement_expression(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXPR);

	int reg = codegen_expression(ctx, stat->stat_expr.expr);
	context_reg_free(ctx, reg);
}

static void codegen_statement_decl_var(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DECL_VAR);
}

static void codegen_statement_decl_func(Context * ctx, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DECL_FUNC);

	code_append(ctx, "%s:\n", stat->stat_func.name);
	//code_append(ctx, "sub rsp, 8; align stack to multiple of 16 bytes\n");

	ctx->indent++;
	codegen_statement(ctx, stat->stat_func.body);
	ctx->indent--;

	//code_append(ctx, "add rsp, 8; align stack to multiple of 16 bytes\n");
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

		if (reg_return != EAX) {
			code_append(ctx, "mov eax, %s ; Return via eax\n", reg_names[reg_return]);
		}

		context_reg_free(ctx, reg_return);
	} else {
		code_append(ctx, "mov eax, 0\n");
	}

	code_append(ctx, "ret\n");
}

static void codegen_statement(Context * ctx, AST_Statement const * stat) {
	switch (stat->type) {
		case AST_STATEMENT_NOOP: codegen_statement_noop      (ctx, stat); break;
		case AST_STATEMENTS:     codegen_statement_statements(ctx, stat); break;

		case AST_STATEMENT_EXPR: codegen_statement_expression(ctx, stat); break;

		case AST_STATEMENT_DECL_VAR:  codegen_statement_decl_var (ctx, stat); break;
		case AST_STATEMENT_DECL_FUNC: codegen_statement_decl_func(ctx, stat); break;

		case AST_STATEMENT_IF:    codegen_statement_if   (ctx, stat); break;
		case AST_STATEMENT_WHILE: codegen_statement_while(ctx, stat); break;

		case AST_STATEMENT_BREAK:    codegen_statement_break   (ctx, stat); break;
		case AST_STATEMENT_CONTINUE: codegen_statement_continue(ctx, stat); break;
		case AST_STATEMENT_RETURN:   codegen_statement_return  (ctx, stat); break;
	}
}

char const * codegen_program(AST_Statement const * program) {
	Context ctx;
	context_init(&ctx);

	char const asm_init[] =
		"EXTERN MessageBoxA: PROC\n"
		"EXTERN GetForegroundWindow: PROC\n\n"

		"GLOBAL main\n\n"

		"SECTION .data\n"
		"hello_msg db \"Hello world\", 0\n"
		"info_msg  db \"Info\", 0\n\n"

		"SECTION .bss\n"
		"alignb 8\n"
		"a resd 1\n"
		"b resd 1\n"
		"c resd 1\n"
		"d resd 1\n"
		"i resd 1\n"
		"j resd 1\n\n"

		"SECTION .text\n\n";

	code_append(&ctx, asm_init);

	codegen_statement(&ctx, program);

	return ctx.code;
}

//static void test() {
//	char const asm[] =
//		"EXTERN MessageBoxA: PROC\n"
//		"EXTERN GetForegroundWindow: PROC\n"
//
//		".data\n"
//		"hello_msg db \"Hello world\", 0\n"
//		"info_msg  db \"Info\", 0\n"
//
//		".code\n"
//		"main PROC\n"
//		"	push rbp ; save frame pointer\n"
//		"	mov rbp, rsp ; fix stack pointer\n"
//		"	sub rsp, 8 * (4 + 2) ; allocate shadow register area + 2 QWORDs for stack alignment\n"
//
//		"	; Get a window handle\n"
//		"	call GetForegroundWindow\n"
//		"	mov rcx, rax\n"
//
//		"	lea rdx, hello_msg\n"
//		"	lea r8,  info_msg\n"
//		"	mov r9, 0 ; MB_OK\n"
//
//		"	and rsp, not 8 ; align stack to 16 bytes prior to API call\n"
//		"	call MessageBoxA\n"
//
//		"	; epilog. restore stack pointer\n"
//		"	mov rsp, rbp\n"
//		"	pop rbp\n"
//
//		"	mov rax, 3\n"
//		"	ret \n"
//		"main ENDP\n"
//
//		"END";
//
//	char const * file_asm = "test.asm";
//	char const * file_exe = "test.exe";
//
//	FILE * file;
//	fopen_s(&file, file_asm, "wb");
//
//	if (file == NULL) abort();
//
//	fwrite(asm, 1, sizeof(asm) - 1, file);
//	fclose(file);
//
//	char const * dir_kernel32 = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\kernel32.lib";
//	char const * dir_user32   = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\user32.lib";
//
//	char const cmd[1024];
//	sprintf_s(cmd, sizeof(cmd), "ml64.exe %s /link /subsystem:windows /defaultlib:\"%s\" /defaultlib:\"%s\" /entry:main", file_asm, dir_kernel32, dir_user32);
//
//	if (system(cmd) != EXIT_SUCCESS) abort();
//
//	int ret = system(file_exe);
//	printf("Program returned: %i\n", ret);
//}

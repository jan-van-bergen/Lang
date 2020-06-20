#include "Godegen.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

//typedef struct Context {
//	char const ** variables;
//	char const ** functions;
//} Context;

typedef struct Code {
	char * code;
	int code_len;
	int code_cap;
} Code;

static void code_init(Code * code) {
	const int initial_capacity = 512;

	code->code     = malloc(initial_capacity);
	code->code_len = 0;
	code->code_cap = initial_capacity;
}

static void code_append(Code * code, char const * chars, int char_count) {
	int new_length = code->code_len + char_count;
	if (new_length >= code->code_cap) {
		code->code_cap *= 2;
		code->code = realloc(code->code, code->code_cap);
	}

	memcpy(code->code + code->code_len, chars, char_count);
	code->code_len = new_length;
}

#define CODE_APPEND(code, buf) code_append(code, buf, sizeof(buf) - 1);

static void codegen_expression(Code * code, AST_Expression const * expr);
static void codegen_statement (Code * code, AST_Statement  const * stat);

static void codegen_expression_const(Code * code, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_CONST);

	char asm[128];
	sprintf_s(asm, sizeof(asm), "mov eax, %i\n", expr->expr_const.token.value_int);

	code_append(code, asm, strlen(asm));
}

static void codegen_expression_op_bin(Code * code, AST_Expression const * expr) {
	assert(expr->type == AST_EXPRESSION_OPERATOR_BIN);

	codegen_expression(code, expr->expr_op_bin.expr_left);

	CODE_APPEND(code, "mov ebx, eax\n");
	
	codegen_expression(code, expr->expr_op_bin.expr_right);

	CODE_APPEND(code, "add eax, ebx\n");
}

static void codegen_expression(Code * code, AST_Expression const * expr) {
	switch (expr->type) {
		case AST_EXPRESSION_CONST: codegen_expression_const(code, expr); break;
		//case AST_EXPRESSION_VAR:
		//case AST_EXPRESSION_ASSIGN:
		case AST_EXPRESSION_OPERATOR_BIN: codegen_expression_op_bin(code, expr); break;
		//case AST_EXPRESSION_OPERATOR_PRE:
		//case AST_EXPRESSION_OPERATOR_POST:
		//case AST_EXPRESSION_CALL_FUNC:
	}
}

static void codegen_statement_noop(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_NOOP);

	CODE_APPEND(code, "nop\n");
}

static void codegen_statement_statements(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENTS);

	if (stat->stat_statements.head) codegen_statement(code, stat->stat_statements.head);
	if (stat->stat_statements.cons) codegen_statement(code, stat->stat_statements.cons);
}

static void codegen_statement_expression(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_EXPR);

	codegen_expression(code, stat->stat_expr.expr);
}

static void codegen_statement_decl_var(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DECL_VAR);
}

static void codegen_statement_decl_func(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_DECL_FUNC);
}

static void codegen_statement_if(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_IF);
}

static void codegen_statement_while(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_WHILE);
}

static void codegen_statement_return(Code * code, AST_Statement const * stat) {
	assert(stat->type == AST_STATEMENT_RETURN);

	CODE_APPEND(code, "ret\n");
}

static void codegen_statement(Code * code, AST_Statement const * stat) {
	switch (stat->type) {
		case AST_STATEMENT_NOOP: codegen_statement_noop      (code, stat); break;
		case AST_STATEMENTS:     codegen_statement_statements(code, stat); break;

		case AST_STATEMENT_EXPR: codegen_statement_expression(code, stat); break;

		case AST_STATEMENT_DECL_VAR:  codegen_statement_decl_var (code, stat); break;
		case AST_STATEMENT_DECL_FUNC: codegen_statement_decl_func(code, stat); break;

		case AST_STATEMENT_IF:    codegen_statement_if   (code, stat); break;
		case AST_STATEMENT_WHILE: codegen_statement_while(code, stat); break;

		case AST_STATEMENT_RETURN: codegen_statement_return(code, stat); break;
	}
}

void codegen_program(AST_Statement const * program) {
	Code code;
	code_init(&code);

	char const pre[] =
		"EXTERN MessageBoxA: PROC\n"
		"EXTERN GetForegroundWindow: PROC\n"

		".data\n"
		"hello_msg db \"Hello world\", 0\n"
		"info_msg  db \"Info\", 0\n"

		".code\n"
		"main PROC\n";

	code_append(&code, pre, strlen(pre));

	codegen_statement(&code, program);

	char const post[] = "main ENDP\nEND";

	code_append(&code, post, strlen(post) + 1);
	
	char const * file_asm = "codegen.asm";
	char const * file_exe = "codegen.exe";

	FILE * file;
	fopen_s(&file, file_asm, "wb");

	if (file == NULL) abort();

	fwrite(code.code, 1, strlen(code.code), file);
	fclose(file);

	char const * dir_kernel32 = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\kernel32.lib";
	char const * dir_user32   = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\user32.lib";

	char const cmd[1024];
	sprintf_s(cmd, sizeof(cmd), "ml64.exe %s /link /subsystem:windows /defaultlib:\"%s\" /defaultlib:\"%s\" /entry:main", file_asm, dir_kernel32, dir_user32);

	if (system(cmd) != EXIT_SUCCESS) abort();

	int ret = system(file_exe);
	printf("Program returned: %i\n", ret);
}

static void test() {
	char const asm[] =
		"EXTERN MessageBoxA: PROC\n"
		"EXTERN GetForegroundWindow: PROC\n"

		".data\n"
		"hello_msg db \"Hello world\", 0\n"
		"info_msg  db \"Info\", 0\n"

		".code\n"
		"main PROC\n"
		"	push rbp ; save frame pointer\n"
		"	mov rbp, rsp ; fix stack pointer\n"
		"	sub rsp, 8 * (4 + 2) ; allocate shadow register area + 2 QWORDs for stack alignment\n"

		"	; Get a window handle\n"
		"	call GetForegroundWindow\n"
		"	mov rcx, rax\n"

		"	lea rdx, hello_msg\n"
		"	lea r8,  info_msg\n"
		"	mov r9, 0 ; MB_OK\n"

		"	and rsp, not 8 ; align stack to 16 bytes prior to API call\n"
		"	call MessageBoxA\n"

		"	; epilog. restore stack pointer\n"
		"	mov rsp, rbp\n"
		"	pop rbp\n"

		"	mov rax, 3\n"
		"	ret \n"
		"main ENDP\n"

		"END";

	char const * file_asm = "test.asm";
	char const * file_exe = "test.exe";

	FILE * file;
	fopen_s(&file, file_asm, "wb");

	if (file == NULL) abort();

	fwrite(asm, 1, sizeof(asm) - 1, file);
	fclose(file);

	char const * dir_kernel32 = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\kernel32.lib";
	char const * dir_user32   = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\user32.lib";

	char const cmd[1024];
	sprintf_s(cmd, sizeof(cmd), "ml64.exe %s /link /subsystem:windows /defaultlib:\"%s\" /defaultlib:\"%s\" /entry:main", file_asm, dir_kernel32, dir_user32);

	if (system(cmd) != EXIT_SUCCESS) abort();

	int ret = system(file_exe);
	printf("Program returned: %i\n", ret);
}

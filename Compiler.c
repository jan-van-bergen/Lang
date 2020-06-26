#include "Compiler.h"

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>

#include "Lexer.h"
#include "Parser.h"
#include "Godegen.h"

#include "Util.h"

void compile_file(char const * filename, bool show_output) {
	char const * source = read_file(filename);

	Lexer lexer;
	lexer_init(&lexer, source);

	int     token_count = 0;
	int     token_capacity = 1024;
	Token * tokens = malloc(token_capacity * sizeof(Token));

	// Lexing
	while (!lexer_reached_end(&lexer)) {
		lexer_get_token(&lexer, tokens + token_count);
		token_count++;
	}

	tokens[token_count++].type = TOKEN_EOF;

	// Parsing
	Parser parser;
	parser_init(&parser, tokens, token_count);

	AST_Statement * program = parser_parse_program(&parser);
	
	free(tokens);

	//printf("\n\nPretty Print:\n\n");
	//ast_pretty_print(program);

	char const * code = codegen_program(program);

	ast_free_statement(program);

	char const * file_asm = replace_file_extension(filename, "asm");
	char const * file_obj = replace_file_extension(filename, "obj");
	char const * file_exe = replace_file_extension(filename, "exe");

	FILE * file;
	fopen_s(&file, file_asm, "wb");

	if (file == NULL) abort();

	fwrite(code, 1, strlen(code), file);
	fclose(file);

	char const * dir_kernel32 = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\kernel32.lib";
	char const * dir_user32   = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\user32.lib";
	char const * dir_liburcrt = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\ucrt\\x64\\libucrt.lib";

	char const cmd[1024];

	// Assemble
	sprintf_s(cmd, sizeof(cmd), "nasm -f win64 %s -o \"%s\" %s", file_asm, file_obj, show_output ? "" : "> nul");
	if (system(cmd) != EXIT_SUCCESS) abort();

	// Link
	sprintf_s(cmd, sizeof(cmd), "link \"%s\" /out:\"%s\" /subsystem:CONSOLE /defaultlib:\"%s\" /defaultlib:\"%s\" /defaultlib:\"%s\" /entry:main %s",
		file_obj,
		file_exe,
		dir_kernel32,
		dir_user32,
		dir_liburcrt,
		show_output ? "" : "> nul"
	);
	if (system(cmd) != EXIT_SUCCESS) abort();

	free(file_asm);
	free(file_obj);
	free(file_exe);
}

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <assert.h>
#include <string.h>

#include "Lexer.h"
#include "Parser.h"
#include "Godegen.h"

static char const * read_file(char const * filename) {
	FILE * f;
	fopen_s(&f, filename, "rb");

	if (f == NULL) {
		printf("Unable to open file %s!\n", filename);
		abort();
	}

	fseek(f, 0, SEEK_END);
	int file_length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char * string = malloc(file_length + 1);
	fread(string, 1, file_length, f);
	string[file_length] = '\0';

	fclose(f);

	return string;
}

int main(int arg_count, char const * args[]) {
	clock_t clock_start = clock();

	char const * filename = "Data\\functions.lang";
	if (arg_count > 1) {
		filename = args[1];
	}

	char const * source = read_file(filename);

	Lexer lexer;
	lexer_init(&lexer, source);

	int   token_count = 0;
	Token tokens[1024];

	// Lexing
	while (!lexer_reached_end(&lexer)) {
		lexer_get_token(&lexer, tokens + token_count);
		token_count++;
	}

	assert(token_count <= sizeof(tokens) / sizeof(Token));

	// Print Tokens
	//for (int i = 0; i < token_count; i++) {
	//	char string[128];
	//	token_to_string(&tokens[i], string, sizeof(string));

	//	printf("%i - %s\n", i, string);
	//}

	tokens[token_count++].type = TOKEN_EOF;

	// Parsing
	Parser parser;
	parser_init(&parser, tokens, token_count);

	AST_Statement * program = parser_parse_program(&parser);

	printf("\n\nPretty Print:\n\n");
	ast_pretty_print(program);

	char const * code = codegen_program(program);
	
	char const * file_extension = strstr(filename, ".lang");
	int filename_len;
	if (file_extension) {
		filename_len = file_extension - filename;
	} else {
		filename_len = strlen(filename);
	}

	char const * file_asm = malloc(filename_len + 5);
	char const * file_obj = malloc(filename_len + 5);
	char const * file_exe = malloc(filename_len + 5);

	memcpy_s(file_asm, filename_len + 5, filename, filename_len);
	memcpy_s(file_obj, filename_len + 5, filename, filename_len);
	memcpy_s(file_exe, filename_len + 5, filename, filename_len);

	memcpy_s(file_asm + filename_len, 5, ".asm", 5);
	memcpy_s(file_obj + filename_len, 5, ".obj", 5);
	memcpy_s(file_exe + filename_len, 5, ".exe", 5);

	FILE * file;
	fopen_s(&file, file_asm, "wb");

	if (file == NULL) abort();

	fwrite(code, 1, strlen(code), file);
	fclose(file);

	char const * dir_kernel32 = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\kernel32.lib";
	char const * dir_user32   = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\user32.lib";

	char const cmd[1024];

	// Assemble
	sprintf_s(cmd, sizeof(cmd), "nasm -f win64 %s -o \"%s\"", file_asm, file_obj);
	if (system(cmd) != EXIT_SUCCESS) abort();

	// Link
	sprintf_s(cmd, sizeof(cmd), "link \"%s\" /out:\"%s\" /subsystem:CONSOLE /defaultlib:\"%s\" /defaultlib:\"%s\" /entry:main", file_obj, file_exe, dir_kernel32, dir_user32);
	if (system(cmd) != EXIT_SUCCESS) abort();

	int ret = system(file_exe);
	printf("Program returned: %i\n", ret);
	
	clock_t clock_end = clock();

	int time = (clock_end - clock_start) * 1000 / CLOCKS_PER_SEC;

	printf("Completed in %i ms\n", time);
	getchar();

	return EXIT_SUCCESS;
}

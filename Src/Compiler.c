#include "Compiler.h"

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>

#include "Lexer.h"
#include "Parser.h"
#include "Codegen.h"

#include "Util.h"
#include "Error.h"

void compile_file(char const * filename, Compiler_Config const * config) {
	char const * source = read_file(filename);

	// Lexing phase
	Lexer lexer;
	lexer_init(&lexer, source);

	lexer_lex(&lexer);

	free(source);

	// Parsing phase
	Parser parser;
	parser_init(&parser, lexer.tokens_len, lexer.tokens);

	type_table_init();

	AST_Statement * program = parser_parse_program(&parser);
	
	lexer_free(&lexer);

	//printf("\n\nPretty Print:\n\n");
	//ast_pretty_print(program);

	// Code Generation phase
	char const * code = codegen_program(program, config->output == COMPILER_OUTPUT_EXE);

	ast_free_statement(program);
	
	type_table_free();

	// Output
	char const * file_asm = replace_file_extension(filename, "asm");
	char const * file_obj = replace_file_extension(filename, "obj");
	char const * file_exe = replace_file_extension(filename, "exe");
	char const * file_lib = replace_file_extension(filename, "lib");

	FILE * file;
	fopen_s(&file, file_asm, "wb");

	if (file == NULL) {
		printf("ERROR: Unable to open asm file '%s' for writing!\n", file_asm);
		error(ERROR_ASSEMBLER);
	}

	fwrite(code, 1, strlen(code), file);
	fclose(file);

	free(code);

	char const * loc_kernel32 = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\kernel32.lib";
	
	char const cmd[1024];

	// Assemble
	sprintf_s(cmd, sizeof(cmd), "nasm -f win64 \"%s\" -o \"%s\" -g", file_asm, file_obj);
	if (system(cmd) != EXIT_SUCCESS) error(ERROR_ASSEMBLER);

	switch (config->output) {
		case COMPILER_OUTPUT_LIB: {
			// Make .lib
			sprintf_s(cmd, sizeof(cmd), "lib %s /out:\"%s\" /nologo", file_obj, file_lib);
			break;
		}

		case COMPILER_OUTPUT_EXE: {
			// Link
			int cmd_offset = sprintf_s(cmd, sizeof(cmd), "link \"%s\" \"Examples\\stdlib.obj\" /out:\"%s\" /subsystem:console /entry:_start /debug /nologo /defaultlib:\"%s\" ",
				file_obj,
				file_exe,
				loc_kernel32
			);

			for (int i = 0; i < config->lib_count; i++) {
				cmd_offset += sprintf_s(cmd + cmd_offset, sizeof(cmd) - cmd_offset, " /defaultlib:\"%s\" ", config->libs[i]);
			}

			break;
		}

		default: error(ERROR_UNKNOWN);
	}

	if (system(cmd) != EXIT_SUCCESS) {
		puts(cmd);	
		error(ERROR_LINKER);
	}
	free(file_asm);
	free(file_obj);
	free(file_exe);
	free(file_lib);
}

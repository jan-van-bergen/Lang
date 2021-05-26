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

void config_add_lib(Compiler_Config * config, char const * lib_name) {
	if (config->lib_count < MAX_NUM_LIBS) {
		config->libs[config->lib_count++] = lib_name;
	} else {
		printf("ERROR: Too many libraries given, only %i are supported!\n", MAX_NUM_LIBS);
		error_internal();
	}
}

void compile_file(char const * filename, Compiler_Config const * config) {
	char const * source = read_file(filename);

	// Lexing phase
	Lexer lexer;
	lexer_init(&lexer, source);

	lexer_lex(&lexer);

	mem_free(source);

	// Parsing phase
	Parser parser;
	parser_init(&parser, lexer.tokens_len, lexer.tokens);

	type_table_init();

	AST_Statement * program = parser_parse_program(&parser);
	
	lexer_mem_free(&lexer);

	//printf("\n\nPretty Print:\n\n");
	//ast_pretty_print(program);

	// Code Generation phase
	char const * code = codegen_program(program, config->output == COMPILER_OUTPUT_EXE);

	ast_free_statement(program);
	
	type_table_mem_free();

	// Output
	char const * file_asm = replace_file_extension(filename, "asm");
	char const * file_obj = replace_file_extension(filename, "obj");
	char const * file_exe = replace_file_extension(filename, "exe");
	char const * file_lib = replace_file_extension(filename, "lib");
	char const * file_dll = replace_file_extension(filename, "dll");

	FILE * file;
	fopen_s(&file, file_asm, "wb");

	if (file == NULL) {
		error_set_line(-1);
		error(ERROR_ASSEMBLER, "Unable to open asm file '%s' for writing!\n", file_asm);
	}

	fwrite(code, 1, strlen(code), file);
	fclose(file);

	mem_free(code);

	char const * lib_path = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64";
	
	char const cmd[1024] = { 0 };

	// Assemble
	sprintf_s(cmd, sizeof(cmd), "nasm -f win64 \"%s\" -o \"%s\" -g -Werror", file_asm, file_obj);
	if (system(cmd) != EXIT_SUCCESS) error(ERROR_ASSEMBLER, "NASM Failed!");

	int cmd_offset = 0;

	switch (config->output) {
		case COMPILER_OUTPUT_EXE: {
			cmd_offset = sprintf_s(cmd, sizeof(cmd), "link \"%s\" /out:\"%s\" /subsystem:console /entry:__start /debug /nologo /opt:ref /libpath:\"%s\" kernel32.lib",
				file_obj,
				file_exe,
				lib_path
			);
			break;
		}
		
		case COMPILER_OUTPUT_LIB: {
			cmd_offset = sprintf_s(cmd, sizeof(cmd), "lib %s /out:\"%s\" /nologo /libpath:\"%s\"", file_obj, file_lib, lib_path);
			break;
		}

		case COMPILER_OUTPUT_DLL: {
			cmd_offset = sprintf_s(cmd, sizeof(cmd), "link /dll %s /out:\"%s\" /entry:DllMain /nologo /opt:ref /libpath:\"%s\" kernel32.lib", file_obj, file_dll, lib_path);
			break;
		}

		default: error_internal();
	}
	
	for (int i = 0; i < config->lib_count; i++) {
		cmd_offset += sprintf_s(cmd + cmd_offset, sizeof(cmd) - cmd_offset, " \"%s\" ", config->libs[i]);
	}

	if (system(cmd) != EXIT_SUCCESS) {
		puts(cmd);	
		error(ERROR_LINKER, "Linker Failed!");
	}

	mem_free(file_asm);
	mem_free(file_obj);
	mem_free(file_exe);
	mem_free(file_lib);
	mem_free(file_dll);
}

#pragma once

typedef enum Compiler_Output {
	COMPILER_OUTPUT_EXE,
	COMPILER_OUTPUT_LIB,
	COMPILER_OUTPUT_DLL
} Compiler_Output;

#define MAX_NUM_LIBS 16

typedef struct Compiler_Config {
	Compiler_Output output;

	char const * libs[MAX_NUM_LIBS];
	int          lib_count;
} Compiler_Config;

void config_add_lib(Compiler_Config * config, char const * lib_name);

void compile_file(char const * filename, Compiler_Config const * config);

#pragma once

typedef enum Compiler_Output {
	COMPILER_OUTPUT_EXE,
	COMPILER_OUTPUT_LIB
} Compiler_Output;

typedef struct Compiler_Config {
	Compiler_Output output;

	char const * libs[16];
	int          lib_count;
} Compiler_Config;

void compile_file(char const * filename, Compiler_Config const * config);

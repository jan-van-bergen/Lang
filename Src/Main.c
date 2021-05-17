#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Compiler.h"

#include "Util.h"
#include "Error.h"

#define RUN_PROGRAM 0 // For debugging only

bool match_arg(char const ** arg, char const * target) {
	int len = strlen(target);

	bool match = strncmp(*arg, target, len) == 0;
	if (match) *arg += len;

	return match;
}

int main(int arg_count, char const * args[]) {
	char const * filename = "Examples\\constdiv.lang";

	Compiler_Config config = {
		.output    = COMPILER_OUTPUT_EXE,
		.lib_count = 0
	};

	// Parse args
	if (arg_count > 1) {
		filename = args[1];

		for (int i = 2; i < arg_count; i++) {
			char const ** arg = &args[i];

			if (match_arg(arg, "/lib:")) {
				config_add_lib(&config, *arg);
			} else if (match_arg(arg, "/out:")) {
				if (strncmp(*arg, "exe", 3) == 0) {
					config.output = COMPILER_OUTPUT_EXE;
				} else if (strncmp(*arg, "lib", 3) == 0) {
					config.output = COMPILER_OUTPUT_LIB;
				} else if (strncmp(*arg, "dll", 3) == 0) {
					config.output = COMPILER_OUTPUT_DLL;
				} else {
					printf("WARNING: Invalid output type '%s' provided. Only 'exe' and 'lib' are valid\n", *arg);
				}
			} else {
				printf("WARNING: Unable to recognize argument '%s'\n", *arg);
			}
		}
	}

	compile_file(filename, &config);

#if RUN_PROGRAM
	char const * file_exe = replace_file_extension(filename, "exe");

	int ret = system(file_exe);
	printf("Program returned: %i (%x)\n", ret, ret);
	
	mem_free(file_exe);

	__debugbreak();

	return ERROR_INTERNAL;
#endif

	return ERROR_SUCCESS;
}

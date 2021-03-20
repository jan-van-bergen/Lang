#include <stdio.h>
#include <stdlib.h>

#include "Compiler.h"

#include "Util.h"
#include "Error.h"

#define RUN_PROGRAM 0 // For debugging only

int main(int arg_count, char const * args[]) {
	char const * filename = "Examples\\factorial.lang";
	if (arg_count > 1) {
		filename = args[1];
	}

	compile_file(filename, true);

#if RUN_PROGRAM
	char const * file_exe = replace_file_extension(filename, "exe");

	int ret = system(file_exe);
	printf("Program returned: %i\n", ret);
	
	free(file_exe);

	__debugbreak();
#endif

	return ERROR_SUCCESS;
}

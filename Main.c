#include <stdio.h>
#include <stdlib.h>

#include "Compiler.h"
#include "Test.h"

#include "Util.h"

#define RUN_TESTS 1

int main(int arg_count, char const * args[]) {
	char const * filename = "Data\\list.lang";
	if (arg_count > 1) {
		filename = args[1];
	}

	compile_file(filename, true);

	char const * file_exe = replace_file_extension(filename, "exe");

	int ret = system(file_exe);
	printf("Program returned: %i\n", ret);

	free(file_exe);

#if RUN_TESTS
	run_tests();
#endif

	puts("Press any key to continue...");
	getchar();

	return EXIT_SUCCESS;
}

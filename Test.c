#include "Test.h"

#include <stdio.h>
#include <stdlib.h>

#include "Compiler.h"

#include "Util.h"

static void run_test(char const * filename, int expected_exit_code) {
	compile_file(filename);

	char const * filename_exe = replace_file_extension(filename, "exe");

	int exit_code = system(filename_exe);

	if (exit_code == expected_exit_code) {
		printf("Testcase SUCCESS: '%s' Exit code was %i\n", filename, exit_code);
	} else {
		printf("Testcase FAILED:  '%s' Exit code was %i, expected %i.\n", filename, exit_code, expected_exit_code);

		__debugbreak();
	}

	free(filename_exe);
}

void run_tests() {
	run_test("Data\\calling_convention.lang", 21);
	run_test("Data\\code.lang",                3);
	run_test("Data\\extern.lang",             13);
	run_test("Data\\functions.lang",          14);
	run_test("Data\\pointer.lang",            21);
	run_test("Data\\double_pointer.lang",      2);
	run_test("Data\\factorial.lang",           1);
	run_test("Data\\div.lang",                 1);
	run_test("Data\\mod.lang",                 2);
}

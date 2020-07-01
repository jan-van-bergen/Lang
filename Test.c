#include "Test.h"

#include <stdio.h>
#include <stdlib.h>

#include "Compiler.h"

#include "Util.h"

static void run_test(char const * filename, int expected_exit_code, char const * expected_output) {
	compile_file(filename, false);

	char const * filename_exe = replace_file_extension(filename, "exe");
	char const * file_out = "test_out.txt";

	char cmd[1024];
	sprintf_s(cmd, sizeof(cmd), "%s > %s", filename_exe, file_out);
	
	free(filename_exe);

	int exit_code = system(cmd);

	char const * output = read_file(file_out);

	if (exit_code == expected_exit_code && strcmp(output, expected_output) == 0) {
		printf("Testcase SUCCESS: '%s' Exit code was %i, output was: '%s'\n", filename, exit_code, output);
	} else {
		printf("Testcase FAILED:  '%s' Exit code was %i, expected %i.\n", filename, exit_code, expected_exit_code);
		printf("Output:   %s\n", output);
		printf("Expected: %s\n", expected_output);

		__debugbreak();
	}

	free(output);
}

void run_tests() {
	puts("Starting tests...");

	run_test("Data\\calling_convention.lang", 21, "");
	run_test("Data\\code.lang",           0xc0de, "");
	run_test("Data\\extern.lang",             13, "Hallo wereld!");
	run_test("Data\\functions.lang",          12, "");
	run_test("Data\\pointer.lang",            21, "");
	run_test("Data\\double_pointer.lang",      2, "");
	run_test("Data\\factorial.lang",           1, "");
	run_test("Data\\div.lang",                 1, "");
	run_test("Data\\mod.lang",                 2, "");
	run_test("Data\\fizzbuzz.lang",            0, "1 2 buzz 4 fizz buzz 7 8 buzz fizz 11 buzz 13 14 fizzbuzz 16 17 buzz 19 fizz ");
	run_test("Data\\logic.lang",               0, "");
	run_test("Data\\heap.lang",         67305985, "BruhTest");
	run_test("Data\\scope.lang",               3, "");
	run_test("Data\\cast.lang",       0x0a0b0c0d, "");
	run_test("Data\\struct.lang",              4, "");
	run_test("Data\\struct_nested.lang",       4, "");
	run_test("Data\\struct_global.lang",      15, "");
}

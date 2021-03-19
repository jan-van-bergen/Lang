#include "Test.h"

#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "Compiler.h"

#include "Util.h"

static int num_tests_total   = 0;
static int num_tests_success = 0;
static int num_tests_failed  = 0;

static void run_test(char const * filename, char const * args, int expected_exit_code, char const * expected_output) {
	num_tests_total++;

	compile_file(filename, false);

	char const * filename_exe = replace_file_extension(filename, "exe");
	char const * file_out = "test_out.txt";

	char cmd[1024];
	sprintf_s(cmd, sizeof(cmd), "%s %s > %s", filename_exe, args, file_out);
	
	free(filename_exe);

	int exit_code = system(cmd);

	char const * output = read_file(file_out);

	if (exit_code == expected_exit_code && strcmp(output, expected_output) == 0) {
		printf("Testcase SUCCESS: '%s' Exit code was %i, output was: '%s'\n", filename, exit_code, output);

		num_tests_success++;
	} else {
		printf("Testcase FAILED:  '%s' Exit code was %i, expected %i.\n", filename, exit_code, expected_exit_code);
		printf("Output:   '%s'\n", output);
		printf("Expected: '%s'\n", expected_output);

		__debugbreak();

		num_tests_failed++;
	}

	free(output);
}

void run_tests() {
	puts("Starting tests...");

	run_test("Examples\\args.lang",               "TEST onespace  twospace           lottaspace", 5, "Examples\\args.exe\nTEST\nonespace\ntwospace\nlottaspace\n");
	run_test("Examples\\calling_convention.lang", "",         11, "");
	run_test("Examples\\code.lang",               "",     0xc0de, "");
	run_test("Examples\\extern.lang",             "",         13, "Hallo wereld!");
	run_test("Examples\\functions.lang",          "",         12, "");
	run_test("Examples\\pointer.lang",            "",         21, "");
	run_test("Examples\\double_pointer.lang",     "",          2, "");
	run_test("Examples\\factorial.lang",          "",          1, "");
	run_test("Examples\\div.lang",                "",          1, "");
	run_test("Examples\\mod.lang",                "",          2, "");
	run_test("Examples\\fizzbuzz.lang",           "",          0, "1 2 buzz 4 fizz buzz 7 8 buzz fizz 11 buzz 13 14 fizzbuzz 16 17 buzz 19 fizz ");
	run_test("Examples\\logic.lang",              "",          0, "");
	run_test("Examples\\heap.lang",               "",   67305985, "BruhTest");
	run_test("Examples\\scope.lang",              "",          3, "");
	run_test("Examples\\cast.lang",               "", 0x0a0b0c0d, "");
	run_test("Examples\\struct.lang",             "",          4, "");
	run_test("Examples\\struct2.lang",            "",         10, "");
	run_test("Examples\\struct_nested.lang",      "",          4, "");
	run_test("Examples\\struct_global.lang",      "",         15, "");
	run_test("Examples\\nested_loops.lang",       "",        541, "2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, ");
	run_test("Examples\\bits.lang",               "",       0xff, "");
	run_test("Examples\\incdec.lang",             "",          2, "");
	run_test("Examples\\file.lang",               "",          0, "");
	run_test("Examples\\array.lang",              "",      85344, "");
	run_test("Examples\\nested_calls.lang",       "",          2, "");
	run_test("Examples\\short_circuit.lang",      "",          0, "");
	run_test("Examples\\float.lang",              "",        288, "");
	run_test("Examples\\float2.lang",             "",          3, "");
	run_test("Examples\\ptrarray.lang",           "",          0, "Hello\n\tworld\nOla\n\tmundo\nBye\n");
	run_test("Examples\\matrix.lang",             "",        249, "2 0 0 2 \n0 3 0 0 \n0 0 4 1 \n0 0 0 1 \n");
	run_test("Examples\\list.lang",               "",          0, "3 1 4 1 5 9 2 6 5 3 5 ");

	assert(num_tests_failed + num_tests_success == num_tests_total);

	puts("");

	if (num_tests_success == num_tests_total) {
		puts("All tests were successful");
	} else {
		printf("WARNING: %i out of %i tests failed!\n", num_tests_failed, num_tests_total);
	}
}

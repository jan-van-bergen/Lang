# from os import system
import subprocess as sp

COMPILER_EXE_DEBUG   = "Debug\\Lang.exe"
COMPILER_EXE_RELEASE = "Release\\lang.exe"

compiler_exe = COMPILER_EXE_DEBUG

# Error codes, should match those in Error.h
class err:
    SUCCESS   = 0
    LEXER     = 1
    PARSER    = 2
    SCOPE     = 3
    TYPECHECK = 4
    CODEGEN   = 5
    ASSEMBLER = 6
    LINKER    = 7
    INTERNAL  = 8

def err_name(errc):
    switcher = {
        err.SUCCESS:   "ERROR_SUCCESS",
        err.LEXER:     "ERROR_LEXER",
        err.PARSER:    "ERROR_PARSER",
        err.SCOPE:     "ERROR_SCOPE",
        err.TYPECHECK: "ERROR_TYPECHECK",
        err.CODEGEN:   "ERROR_CODEGEN",
        err.ASSEMBLER: "ERROR_ASSEMBLER",
        err.LINKER:    "ERROR_LINKER",
        err.INTERNAL:  "ERROR_INTERNAL",
    }
    return switcher.get(errc, "")

class col:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

test_cases = [                       
	("Examples\\stdlib.lang", "/out:lib", 0, "", 1, ""), # should always be first

#	("Examples\\gui.lang", "/lib:Examples\\stdlib.lib /lib:user32.lib /lib:gdi32.lib",                   0, "", 0, "done"),
#	("Examples\\gl.lang",  "/lib:Examples\\stdlib.lib /lib:user32.lib /lib:gdi32.lib /lib:opengl32.lib", 0, "", 0, "done"),

	("Examples\\roman.lang",              "/lib:Examples\\stdlib.lib", 0,             "3549 ",                   0, "MMMDXLIX"),
	("Examples\\chkstk.lang",             "",                          0,             "",                     4096, ""),
	("Examples\\dll.lang",                "/out:dll",                  0,             "",                        1, ""),
	("Examples\\dll_use.lang",            "/lib:Examples\\stdlib.lib", 0,             "",                        0, "Hello!"),
	("Examples\\levenshtein.lang",        "/lib:Examples\\stdlib.lib", 0,             "sitting kitten",          3, ""),
	("Examples\\reinterp.lang",           "/lib:Examples\\stdlib.lib", 0,             "",               0x3f800000, ""),
	("Examples\\float_conv.lang",         "/lib:Examples\\stdlib.lib", 0,             "",                        0, ""),
	("Examples\\montecarlo.lang",         "/lib:Examples\\stdlib.lib", 0,             "",                        3, ""),
	("Examples\\rle.lang",                "/lib:Examples\\stdlib.lib", 0,             "WWWWWWWWWWWWBWWWWWWWWWWWWBBBWWWWWWWWWWWWWWWWWWWWWWWWBWWWWWWWWWWWWWW", 0, "12W1B12W3B24W1B14W"),
	("Examples\\func_ptr.lang",           "",                          0,             "",                       71, ""),
	("Examples\\func_ptr2.lang",          "",                          0,             "",                        6, ""),
	("Examples\\func_ptr3.lang",          "/lib:Examples\\stdlib.lib", 0,             "",                        0, "Value is true\nWaarde is onwaar\n"),
	("Examples\\hash.lang",               "/lib:Examples\\stdlib.lib", 0,             "",                        0, "Resizing...\nResizing...\nResizing...\neen"),
	("Examples\\sudoku.lang",             "/lib:Examples\\stdlib.lib", 0,             "53  7    6  195    98    6 8   6   34  8 3  17   2   6 6    28    419  5    8  79", 1, "Solution:\n534678912\n672195348\n198342567\n859761423\n426853791\n713924856\n961537284\n287419635\n345286179\n"),
	("Examples\\regex.lang",              "/lib:Examples\\stdlib.lib", 0,             ".?a*c*a+bc* aaab",        1, ""),
	("Examples\\regex.lang",              "/lib:Examples\\stdlib.lib", 0,             "a*c*ab acb",              0, ""),
	("Examples\\avl.lang",                "/lib:Examples\\stdlib.lib", 0,             "",                        4, "\n   4\n 2   6\n1 3 5 7\n"),
	("Examples\\mandelbrot.lang",         "/lib:Examples\\stdlib.lib", 0,             "",                        0, ""),
	("Examples\\binary_search.lang",      "/lib:Examples\\stdlib.lib", 0,             "",                       10, ""),
	("Examples\\invert_tree.lang",        "/lib:Examples\\stdlib.lib", 0,             "",                        0, "13 14 10 7 4 6 1 3 8 "),
	("Examples\\array_address.lang",      "",                          0,             "",                     7331, ""),
	("Examples\\ptr_arith.lang",          "",                          0,             "",               0x04030201, ""),
	("Examples\\sort.lang",               "/lib:Examples\\stdlib.lib", 0,             "",                        1, "1, 2, 3, 4, 5, 7, 8, 9"),
	("Examples\\voidstar.lang",           "",                          err.TYPECHECK, "",                        0, ""),
	("Examples\\eval.lang",               "/lib:Examples\\stdlib.lib", 0,             "12 + 2*3",               18, ""),
	("Examples\\matrix.lang",             "/lib:Examples\\stdlib.lib", 0,             "",                      249, "2 0 0 2 \n0 3 0 0 \n0 0 4 1 \n0 0 0 1 \n"),
	("Examples\\struct_array.lang",       "",                          0,             "",                      123, ""),
	("Examples\\linked_list.lang",        "/lib:Examples\\stdlib.lib", 0,             "",                       10, ""),
	("Examples\\hello.lang",              "/lib:Examples\\stdlib.lib", 0,             "",                       12, "Hello world!"),
	("Examples\\type_infer.lang",         "",                          0,             "",                      272, ""),
	("Examples\\args.lang",               "",                          0,             "TEST o  t    l",          5, "Examples\\args.exe\nTEST\no\nt\nl\n"),
	("Examples\\calling_convention.lang", "",                          0,             "",                       11, ""),
	("Examples\\code.lang",               "",                          0,             "",                   0xc0de, ""),
	("Examples\\extern.lang",             "/lib:Examples\\stdlib.lib", 0,             "",                       13, "Hallo wereld!"),
	("Examples\\functions.lang",          "",                          0,             "",                       12, ""),
	("Examples\\pointer.lang",            "",                          0,             "",                       21, ""),
	("Examples\\double_pointer.lang",     "",                          0,             "",                        2, ""),
	("Examples\\factorial.lang",          "",                          0,             "",                        1, ""),
	("Examples\\div.lang",                "",                          0,             "",                        1, ""),
	("Examples\\mod.lang",                "",                          0,             "",                        2, ""),
	("Examples\\fizzbuzz.lang",           "",                          0,             "",                        0, "1 2 buzz 4 fizz buzz 7 8 buzz fizz 11 buzz 13 14 fizzbuzz 16 17 buzz 19 fizz "),
	("Examples\\logic.lang",              "",                          0,             "",                        0, ""),
	("Examples\\heap.lang",               "",                          0,             "",                 67305985, "BruhTest"),
	("Examples\\scope.lang",              "",                          0,             "",                        3, ""),
	("Examples\\scope2.lang",             "",                          0,             "",                       32, ""),
	("Examples\\cast.lang",               "",                          0,             "",               0x0a0b0c0d, ""),
	("Examples\\struct.lang",             "",                          0,             "",                        4, ""),
	("Examples\\struct2.lang",            "",                          0,             "",                       10, ""),
	("Examples\\struct_assign.lang",      "",                          0,             "",                     5678, ""),
	("Examples\\struct_nested.lang",      "",                          0,             "",                        4, ""),
	("Examples\\struct_global.lang",      "",                          0,             "",                       15, ""),
	("Examples\\struct_invalid.lang",     "",                          err.SCOPE,     "",                        0, ""),
	("Examples\\nested_loops.lang",       "",                          0,             "",                      541, "2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, "),
	("Examples\\bits.lang",               "",                          0,             "",                     0xff, ""),
	("Examples\\incdec.lang",             "",                          0,             "",                        2, ""),
	("Examples\\file.lang",               "/lib:Examples\\stdlib.lib", 0,             "",                        0, "test123"),
	("Examples\\array.lang",              "",                          0,             "",                    85344, ""),
	("Examples\\nested_calls.lang",       "",                          0,             "",                       13, ""),
	("Examples\\short_circuit.lang",      "",                          0,             "",                        0, ""),
	("Examples\\float.lang",              "",                          0,             "",                      288, ""),
	("Examples\\float2.lang",             "",                          0,             "",                        3, ""),
	("Examples\\float3.lang",             "",                          0,             "",                       10, ""),
	("Examples\\ptrarray.lang",           "",                          0,             "",                        0, "Hello\n\tworld\nOla\n\tmundo\nBye\n"),
	("Examples\\list.lang",               "",                          0,             "",                       16, "3 1 4 1 5 9 2 6 5 3 5 "),
	("Examples\\error_lexing.lang",       "",                          err.LEXER,     "",                        0, ""),
	("Examples\\error_parser.lang",       "",                          err.PARSER,    "",                        0, ""),
	("Examples\\error_scope.lang",        "",                          err.SCOPE,     "",                        0, ""),
	("Examples\\error_typecheck.lang",    "",                          err.TYPECHECK, "",                        0, ""),
	("Examples\\error_linking.lang",      "",                          err.LINKER,    "",                        0, ""),
	("Examples\\invalid_main.lang",       "",                          err.TYPECHECK, "",                        0, ""),
]

def run_cmd(cmd):
    result = sp.run(cmd, shell=True)
    return result.returncode

num_tests_successfull = 0
num_tests_total       = 0

fail_names = []

for (file, compiler_args, expected_compiler_status, program_args, expected_program_status, expected_program_output) in test_cases:
    num_tests_total += 1

    compiler_cmd = "{} {} {} > compiler_out.txt".format(compiler_exe, file, compiler_args)
    compiler_status = run_cmd(compiler_cmd)

    if (compiler_status != expected_compiler_status):
        print("{}TEST FAILED{}: {} - Unexpected compiler status code".format(col.FAIL, col.ENDC, file))
        print("Expected: {} (0x{:x}) ({})".format(expected_compiler_status, expected_compiler_status, err_name(expected_compiler_status)))
        print("Observed: {} (0x{:x}) ({})".format(compiler_status,          compiler_status,          err_name(compiler_status)))

        print("Compiler Output:")
        print(open("compiler_out.txt", "r").read())

        fail_names.append(file)

        continue

    if (compiler_status == 0):
        program_name = file.replace(".lang", ".exe")

        program_cmd = "{} {} > test_out.txt".format(program_name, program_args)
        program_status = run_cmd(program_cmd)

        program_output = open("test_out.txt", "r").read()

        if (program_status != expected_program_status):
            print("{}TEST FAILED{}: {} - Unexpected program status code".format(col.FAIL, col.ENDC, file))
            print("Expected: {} (0x{:x})".format(expected_program_status, expected_program_status))
            print("Observed: {} (0x{:x})".format(program_status,          program_status))

            print("")
            print("Program Output:")
            print(program_output)

            fail_names.append(file)

            continue

        if (program_output != expected_program_output):
            print("{}TEST FAILED{}: {} - Unexpected program output".format(col.FAIL, col.ENDC, file))
            print("Expected: {}".format(expected_program_output))
            print("Observed: {}".format(program_output))

            fail_names.append(file)

            continue

    print("{}TEST SUCCESS{}: {}".format(col.OKGREEN, col.ENDC, file))

    num_tests_successfull += 1

print()
if (num_tests_successfull == num_tests_total):
    print("{}{}/{} tests were successful{}".format(col.OKGREEN, num_tests_successfull, num_tests_total, col.ENDC))
else:
    print("{}{}/{} tests were successful{}".format(col.FAIL, num_tests_successfull, num_tests_total, col.ENDC))
    print("Failed:")

    for name in fail_names:
        print(name)

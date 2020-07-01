#pragma once
#include "AST.h"

char const * codegen_program(AST_Statement const * program, AST_Def_Func ** function_defs, int function_def_count);

#pragma once
#include "AST.h"

char const * codegen_program(AST_Statement const * program, AST_Decl_Func ** function_decls, int function_decl_count);

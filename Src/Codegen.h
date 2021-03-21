#pragma once
#include "AST.h"

char const * codegen_program(AST_Statement const * program, bool needs_main);

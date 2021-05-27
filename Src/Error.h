#pragma once
#include "Util.h"

#include <stdarg.h>

typedef enum Error {
	ERROR_SUCCESS = 0,
	ERROR_LEXER,
	ERROR_PARSER,
	ERROR_SCOPE,
	ERROR_TYPECHECK,
	ERROR_CODEGEN,
	ERROR_ASSEMBLER,
	ERROR_LINKER,
	ERROR_INTERNAL   
} Error;

NO_RETURN void error (Error error, char const * msg, ...);
NO_RETURN void errorv(Error error, char const * msg, va_list args);

NO_RETURN void error_internal();

void error_set_line(int line);

#pragma once

typedef enum Error {
	ERROR_SUCCESS = 0,
	ERROR_LEXER,
	ERROR_PARSER,
	ERROR_SCOPE,
	ERROR_TYPECHECK,
	ERROR_CODEGEN,
	ERROR_ASSEMBLER,
	ERROR_LINKER,
	ERROR_UNKNOWN   
} Error;

__declspec(noreturn) void error(Error error);

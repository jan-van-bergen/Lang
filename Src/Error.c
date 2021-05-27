#include "Error.h"

#include <stdio.h>
#include <stdlib.h>

static int error_line = -1;

void error(Error error, char const * msg, ...) {
	va_list args;
	va_start(args, msg);
	errorv(error, msg, args);
	va_end(args);
}

NO_RETURN void errorv(Error error, char const * msg, va_list args) {
	char fmt[1024];
	if (error_line == -1) {
		sprintf_s(fmt, sizeof(fmt), "ERROR: %s", msg);
	} else {
		sprintf_s(fmt, sizeof(fmt), "ERROR at line %i: %s", error_line, msg);
	}
	vprintf(fmt, args);
	
	exit(error);
}

NO_RETURN void error_internal() {
	error(ERROR_INTERNAL, "Internal Compiler Error!\n");
}

void error_set_line(int line) {
	error_line = line;
}

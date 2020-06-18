#include <stdio.h>
#include <stdlib.h>

#include "Lexer.h"

char const * source = "i := 5;\nbruh: = \"Hello World!\"; \n      \nprint(bruh)";

int main(int arg_count, char const ** args) {
	Lexer lexer;
	lexer_init(&lexer, source);

	int   token_count = 0;
	Token tokens[32];

	for (int i = 0; i < 32; i++) {
		bool done = lexer_next_token(&lexer, tokens + i);
		if (done) break;

		token_count++;
	}

	return EXIT_SUCCESS;
}

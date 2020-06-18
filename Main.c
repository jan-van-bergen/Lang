#include <stdio.h>
#include <stdlib.h>

#include "Lexer.h"

char const * source = "i := 5;\nbruh: = \"Hello World!\"; \n      \nprint(bruh, 3);\nif(i==3){print(\"i is 3\"); } else { print(\"i is not 3\"); }";

int main(int arg_count, char const ** args) {
	Lexer lexer;
	lexer_init(&lexer, source);

	int   token_count = 0;
	Token tokens[64];

	while (!lexer_reached_end(&lexer)) {
		lexer_next_token(&lexer, tokens + token_count);
		token_count++;
	}

	for (int i = 0; i < token_count; i++) {
		char string[128];
		token_to_string(&tokens[i], string, sizeof(string));

		printf("%i - %s\n", i, string);
	}

	return EXIT_SUCCESS;
}

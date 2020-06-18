#include <stdio.h>
#include <stdlib.h>

#include "Lexer.h"

static char const * read_file(char const * filename) {
	FILE * f;
	fopen_s(&f, filename, "rb");

	if (f == NULL) {
		printf("Unable to open file %s!\n", filename);
		abort();
	}

	fseek(f, 0, SEEK_END);
	int file_length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char * string = malloc(file_length + 1);
	fread(string, 1, file_length, f);
	string[file_length] = '\0';

	fclose(f);

	return string;
}

int main(int arg_count, char const * args[]) {
	char const * filename = "Data/test.lang";
	if (arg_count > 1) {
		filename = args[1];
	}

	char const * source = read_file(filename);

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

	free(source);

	return EXIT_SUCCESS;
}

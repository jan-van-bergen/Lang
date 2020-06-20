#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <assert.h>

#include "Lexer.h"
#include "Parser.h"
#include "Godegen.h"

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
	clock_t clock_start = clock();

	char const * filename = "Data/code.lang";
	if (arg_count > 1) {
		filename = args[1];
	}

	char const * source = read_file(filename);

	Lexer lexer;
	lexer_init(&lexer, source);

	int   token_count = 0;
	Token tokens[1024];

	// Lexing
	while (!lexer_reached_end(&lexer)) {
		lexer_get_token(&lexer, tokens + token_count);
		token_count++;
	}

	assert(token_count <= sizeof(tokens) / sizeof(Token));

	// Print Tokens
	//for (int i = 0; i < token_count; i++) {
	//	char string[128];
	//	token_to_string(&tokens[i], string, sizeof(string));

	//	printf("%i - %s\n", i, string);
	//}

	tokens[token_count++].type = TOKEN_EOF;

	// Parsing
	Parser parser;
	parser_init(&parser, tokens, token_count);

	AST_Statement * program = parser_parse_program(&parser);

	printf("\n\nPretty Print:\n\n");
	ast_pretty_print(program);

	codegen_program(program);

	clock_t clock_end = clock();

	int time = (clock_end - clock_start) * 1000 / CLOCKS_PER_SEC;

	printf("Completed in %i ms\n", time);

	return EXIT_SUCCESS;
}

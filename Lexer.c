#include "Lexer.h"

#include <string.h>
#include <assert.h>

#include <stdlib.h>

void lexer_init(Lexer * lexer, char const * source) {
	lexer->source_len = strlen(source);
	lexer->source     = source;

	lexer->index = 0;
}

bool lexer_reached_end(Lexer const * lexer) {
	return lexer_peek(lexer) == '\0' || lexer->index == lexer->source_len;
}

char lexer_peek(Lexer const * lexer) {
	assert(lexer->index >= 0 && lexer->index <= lexer->source_len);

	return lexer->source[lexer->index];
}

char lexer_next(Lexer * lexer) {
	return lexer->source[++lexer->index];
}

bool lexer_is_whitespace(Lexer const * lexer) {
	char curr = lexer_peek(lexer);

	return curr == ' ' || curr == '\t' || curr == '\n';
}

//static char * char_to_str(char c) {
//	char * str = malloc(2);
//	str[0] = c;
//	str[1] = NULL;
//
//	return str;
//}

bool lexer_next_token(Lexer * lexer, Token * token) {
	if (lexer_reached_end(lexer)) return true;

	while (lexer_is_whitespace(lexer)) lexer_next(lexer);

	char curr = lexer_peek(lexer);

	if (curr == '"') {
		curr = lexer_next(lexer); // Consume "

		int index_str_start = lexer->index;
			
		while (curr != '"') curr = lexer_next(lexer);
			
		int index_str_end = lexer->index;

		int    str_len = index_str_end - index_str_start;
		char * str     = malloc(str_len + 1);

		memcpy(str, lexer->source + index_str_start, index_str_end - index_str_start);
		str[str_len] = '\0';

		token->type      = TOKEN_LITERAL_STRING;
		token->value_str = str;

		lexer_next(lexer);

		return false;
	}

	if (isdigit(curr)) {
		token->type      = TOKEN_LITERAL_INT;
		token->value_int = curr - '0';

		lexer_next(lexer);

		return false;
	}

	switch (curr) {
		case '(': token->type = TOKEN_PARENTESES_OPEN;  lexer_next(lexer); return false;
		case ')': token->type = TOKEN_PARENTESES_CLOSE; lexer_next(lexer); return false;

		case ',': token->type = TOKEN_COMMA;     lexer_next(lexer); return false;
		case ':': token->type = TOKEN_COLON;     lexer_next(lexer); return false;
		case '=': token->type = TOKEN_EQUALS;    lexer_next(lexer); return false;
		case ';': token->type = TOKEN_SEMICOLON; lexer_next(lexer); return false;
	}

	int index_str_start = lexer->index;
		
	lexer_next(lexer); // Consume first letter of identifier

	while (isalnum(curr)) curr = lexer_next(lexer);
			
	int index_str_end = lexer->index;

	int    str_len = index_str_end - index_str_start;
	char * str     = malloc(str_len + 1);

	memcpy(str, lexer->source + index_str_start, index_str_end - index_str_start);
	str[str_len] = '\0';

	token->type = TOKEN_IDENTIFIER;
	token->value_str = str;
		
	return false;
}

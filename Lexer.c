#include "Lexer.h"

#include <string.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

void token_to_string(Token const * token, char * string, int string_size) {
	switch (token->type) {
		case TOKEN_IDENTIFIER:     sprintf_s(string, string_size, "%s",     token->value_str); return;
		case TOKEN_LITERAL_INT:    sprintf_s(string, string_size, "%i",     token->value_int); return;
		case TOKEN_LITERAL_STRING: sprintf_s(string, string_size, "\"%s\"", token->value_str); return;

		case TOKEN_KEYWORD_IF:	   strcpy_s(string, string_size, "if");		return;
		case TOKEN_KEYWORD_ELSE:   strcpy_s(string, string_size, "else");	return;
		case TOKEN_KEYWORD_FOR:	   strcpy_s(string, string_size, "for");	return;
		case TOKEN_KEYWORD_WHILE:  strcpy_s(string, string_size, "while");	return;
		case TOKEN_KEYWORD_STRUCT: strcpy_s(string, string_size, "struct"); return;
		
		case TOKEN_PARENTESES_OPEN:  strcpy_s(string, string_size, "("); return;
		case TOKEN_PARENTESES_CLOSE: strcpy_s(string, string_size, ")"); return;

		case TOKEN_BRACES_OPEN:  strcpy_s(string, string_size, "{"); return;
		case TOKEN_BRACES_CLOSE: strcpy_s(string, string_size, "}"); return;

		case TOKEN_OPERATOR_PLUS:	  strcpy_s(string, string_size, "+"); return;
		case TOKEN_OPERATOR_MINUS:	  strcpy_s(string, string_size, "-"); return;
		case TOKEN_OPERATOR_MULTIPLY: strcpy_s(string, string_size, "*"); return;
		case TOKEN_OPERATOR_DIVIDE:   strcpy_s(string, string_size, "/"); return;

		case TOKEN_OPERATOR_EQUALS:	    strcpy_s(string, string_size, "=="); return;
		case TOKEN_OPERATOR_NOT_EQUALS: strcpy_s(string, string_size, "!="); return;

		case TOKEN_ASSIGN: strcpy_s(string, string_size, "="); return;

		case TOKEN_COMMA:	  strcpy_s(string, string_size, ","); return;
		case TOKEN_COLON:	  strcpy_s(string, string_size, ":"); return;
		case TOKEN_SEMICOLON: strcpy_s(string, string_size, ";"); return;

		default: strcpy_s(string, string_size, "Unknown Token"); return;
	}
}

void lexer_init(Lexer * lexer, char const * source) {
	lexer->source_len = strlen(source);
	lexer->source     = source;

	lexer->index = 0;
}

static char lexer_peek(Lexer const * lexer) {
	assert(lexer->index >= 0 && lexer->index <= lexer->source_len);

	return lexer->source[lexer->index];
}

static char lexer_next(Lexer * lexer) {
	return lexer->source[++lexer->index];
}

static bool lexer_is_whitespace(Lexer const * lexer) {
	char curr = lexer_peek(lexer);

	return curr == ' ' || curr == '\t' || curr == '\r' || curr == '\n';
}

// Skips whitespace, tabs, newlines, and comments
static void lexer_skip(Lexer * lexer) {
	bool changed;

	do {
		changed = false;

		while (lexer_is_whitespace(lexer)) {
			lexer_next(lexer);

			changed = true;
		}

		if (lexer_match(lexer, "//")) { 
			int comment_length = 0;

			while (
				lexer->source[lexer->index + comment_length] != '\r' &&
				lexer->source[lexer->index + comment_length] != '\n' &&
				lexer->source[lexer->index + comment_length] != '\0'
			) comment_length++;

			lexer->index += comment_length;

			changed = true;
		}

		if (lexer_match(lexer, "/*")) {
			lexer->index += 2;

			const char * comment_start = lexer->source + lexer->index;
			const char * comment_end   = strstr(comment_start, "*/");

			lexer->index += comment_end - comment_start + 2;

			changed = true;
		}
	} while (changed);
}

bool lexer_reached_end(Lexer const * lexer) {
	lexer_skip(lexer);

	return lexer_peek(lexer) == '\0' || lexer->index == lexer->source_len;
}

static int lexer_match(Lexer const * lexer, char const * target) {
	int index = 0;

	while (lexer->source[lexer->index + index] == target[index]) {
		index++;

		if (target[index] == '\0') return index;
	}

	return 0;
}

void lexer_next_token(Lexer * lexer, Token * token) {
	lexer_skip(lexer);

	char curr = lexer_peek(lexer);
	
	if (isdigit(curr)) {
		token->type      = TOKEN_LITERAL_INT;
		token->value_int = curr - '0';

		lexer_next(lexer);

		return;
	}

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

		return;
	}
	
	int match_length = 0;
	if (match_length = lexer_match(lexer, "if"))     { token->type = TOKEN_KEYWORD_IF;     lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "else"))   { token->type = TOKEN_KEYWORD_ELSE;   lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "for"))    { token->type = TOKEN_KEYWORD_FOR;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "while"))  { token->type = TOKEN_KEYWORD_WHILE;  lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "struct")) { token->type = TOKEN_KEYWORD_STRUCT; lexer->index += match_length; return; }

	if (match_length = lexer_match(lexer, "==")) { token->type = TOKEN_OPERATOR_EQUALS;     lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "!=")) { token->type = TOKEN_OPERATOR_NOT_EQUALS; lexer->index += match_length; return; }

	switch (curr) {
		case '(': token->type = TOKEN_PARENTESES_OPEN;  lexer_next(lexer); return;
		case ')': token->type = TOKEN_PARENTESES_CLOSE; lexer_next(lexer); return;

		case '{': token->type = TOKEN_BRACES_OPEN;  lexer_next(lexer); return;
		case '}': token->type = TOKEN_BRACES_CLOSE; lexer_next(lexer); return;

		case '+': token->type = TOKEN_OPERATOR_PLUS; 	 lexer_next(lexer); return;
		case '-': token->type = TOKEN_OPERATOR_MINUS; 	 lexer_next(lexer); return;
		case '*': token->type = TOKEN_OPERATOR_MULTIPLY; lexer_next(lexer); return; 
		case '/': token->type = TOKEN_OPERATOR_DIVIDE;   lexer_next(lexer); return;

		case '=': token->type = TOKEN_ASSIGN; lexer_next(lexer); return;

		case ',': token->type = TOKEN_COMMA;     lexer_next(lexer); return;
		case ':': token->type = TOKEN_COLON;     lexer_next(lexer); return;
		case ';': token->type = TOKEN_SEMICOLON; lexer_next(lexer); return;
	}

	// Token will be identifier

	int index_str_start = lexer->index;
		
	curr = lexer_next(lexer); // Consume first letter of identifier

	while (isalnum(curr)) curr = lexer_next(lexer);
			
	int index_str_end = lexer->index;

	int    str_len = index_str_end - index_str_start;
	char * str     = malloc(str_len + 1);

	memcpy(str, lexer->source + index_str_start, index_str_end - index_str_start);
	str[str_len] = '\0';

	token->type = TOKEN_IDENTIFIER;
	token->value_str = str;
		
	return;
}

#include "Lexer.h"

#include <string.h>
#include <assert.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

void lexer_init(Lexer * lexer, char const * source) {
	lexer->source_len = strlen(source);
	lexer->source     = source;

	lexer->index = 0;
	lexer->line  = 1;
}

static char lexer_peek(Lexer const * lexer) {
	assert(lexer->index >= 0 && lexer->index <= lexer->source_len);

	return lexer->source[lexer->index];
}

static char lexer_next(Lexer * lexer) {
	return lexer->source[++lexer->index];
}

static bool lexer_is_whitespace(Lexer * lexer) {
	char curr = lexer_peek(lexer);

	if (curr == '\n') lexer->line++;

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

			while (!lexer_match(lexer, "*/")) {
				if (lexer_next(lexer) == '\n') {
					lexer->line++;
				}
			}

			lexer->index += 2;

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

void lexer_get_token(Lexer * lexer, Token * token) {
	lexer_skip(lexer);

	char curr = lexer_peek(lexer);
	
	token->line = lexer->line;

	if (isdigit(curr)) {
		int value = 0;

		while (isdigit(curr)) {
			value *= 10;
			value += curr - '0';

			curr = lexer_next(lexer);
		}

		token->type      = TOKEN_LITERAL_INT;
		token->value_int = value;

		//lexer_next(lexer);

		return;
	}
	
	int match_length = 0;
	if (match_length = lexer_match(lexer, "true"))  { token->type = TOKEN_LITERAL_BOOL; token->value_char = 1; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "false")) { token->type = TOKEN_LITERAL_BOOL; token->value_char = 0; lexer->index += match_length; return; }

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
	
	// Keywords
	if (match_length = lexer_match(lexer, "let"))      { token->type = TOKEN_KEYWORD_LET;      lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "extern"))   { token->type = TOKEN_KEYWORD_EXTERN;   lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "if"))       { token->type = TOKEN_KEYWORD_IF;       lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "else"))     { token->type = TOKEN_KEYWORD_ELSE;     lexer->index += match_length; return; }
	//if (match_length = lexer_match(lexer, "for"))    { token->type = TOKEN_KEYWORD_FOR;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "while"))    { token->type = TOKEN_KEYWORD_WHILE;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "break"))    { token->type = TOKEN_KEYWORD_BREAK;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "continue")) { token->type = TOKEN_KEYWORD_CONTINUE; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "func"))     { token->type = TOKEN_KEYWORD_FUNC;     lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "return"))   { token->type = TOKEN_KEYWORD_RETURN;   lexer->index += match_length; return; }
	//if (match_length = lexer_match(lexer, "struct")) { token->type = TOKEN_KEYWORD_STRUCT; lexer->index += match_length; return; }

	// Bitshift Operators
	if (match_length = lexer_match(lexer, "<<")) { token->type = TOKEN_OPERATOR_SHIFT_LEFT;  lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, ">>")) { token->type = TOKEN_OPERATOR_SHIFT_RIGHT; lexer->index += match_length; return; }

	// Relational Operators
	if (match_length = lexer_match(lexer, "<=")) { token->type = TOKEN_OPERATOR_LT_EQ; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, ">=")) { token->type = TOKEN_OPERATOR_GT_EQ; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "<"))  { token->type = TOKEN_OPERATOR_LT;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, ">"))  { token->type = TOKEN_OPERATOR_GT;    lexer->index += match_length; return; }
	
	// Equality Operators
	if (match_length = lexer_match(lexer, "==")) { token->type = TOKEN_OPERATOR_EQ; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "!=")) { token->type = TOKEN_OPERATOR_NE; lexer->index += match_length; return; }
	
	// Unary Increment/Decrement Operators
	if (match_length = lexer_match(lexer, "++")) { token->type = TOKEN_OPERATOR_INC; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "--")) { token->type = TOKEN_OPERATOR_DEC; lexer->index += match_length; return; }

	// Aassignment Operators (excluding = see switch statement)
	if (match_length = lexer_match(lexer, "+=")) { token->type = TOKEN_ASSIGN_PLUS;     lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "-=")) { token->type = TOKEN_ASSIGN_MINUS;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "*=")) { token->type = TOKEN_ASSIGN_MULTIPLY; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "/=")) { token->type = TOKEN_ASSIGN_DIVIDE;   lexer->index += match_length; return; }

	if (match_length = lexer_match(lexer, "->")) { token->type = TOKEN_ARROW; lexer->index += match_length; return; }

	switch (curr) {
		case '(': token->type = TOKEN_PARENTESES_OPEN;  lexer_next(lexer); return;
		case ')': token->type = TOKEN_PARENTESES_CLOSE; lexer_next(lexer); return;

		case '{': token->type = TOKEN_BRACES_OPEN;  lexer_next(lexer); return;
		case '}': token->type = TOKEN_BRACES_CLOSE; lexer_next(lexer); return;

		case '+': token->type = TOKEN_OPERATOR_PLUS; 	 lexer_next(lexer); return;
		case '-': token->type = TOKEN_OPERATOR_MINUS; 	 lexer_next(lexer); return;
		case '*': token->type = TOKEN_OPERATOR_MULTIPLY; lexer_next(lexer); return; 
		case '/': token->type = TOKEN_OPERATOR_DIVIDE;   lexer_next(lexer); return;
		case '%': token->type = TOKEN_OPERATOR_MODULO;   lexer_next(lexer); return;

		case '&': token->type = TOKEN_OPERATOR_BITWISE_AND; lexer_next(lexer); return;

		case '=': token->type = TOKEN_ASSIGN; lexer_next(lexer); return;

		case ',': token->type = TOKEN_COMMA;     lexer_next(lexer); return;
		case ':': token->type = TOKEN_COLON;     lexer_next(lexer); return;
		case ';': token->type = TOKEN_SEMICOLON; lexer_next(lexer); return;
	}

	// Token will be identifier

	int index_str_start = lexer->index;
		
	curr = lexer_next(lexer); // Consume first letter of identifier

	while (isalnum(curr) || curr == '_') curr = lexer_next(lexer);
			
	int index_str_end = lexer->index;

	int    str_len = index_str_end - index_str_start;
	char * str     = malloc(str_len + 1);

	memcpy(str, lexer->source + index_str_start, index_str_end - index_str_start);
	str[str_len] = '\0';

	token->type = TOKEN_IDENTIFIER;
	token->value_str = str;
		
	return;
}

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

static int lexer_match(Lexer const * lexer, char const * target) {
	int index = 0;

	while (lexer->source[lexer->index + index] == target[index]) {
		index++;

		if (target[index] == '\0') return index; // Return length of target string
	}

	return 0;
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

bool lexer_match_int_literal(Lexer * lexer) {
	char curr = lexer_peek(lexer);
	
	if (curr != '+' && curr != '-' && !isdigit(curr)) return false;

	bool is_negative = curr == '-';

	int value = 0;

	while (isdigit(curr)) {
		value *= 10;
		value += curr - '0';

		curr = lexer_next(lexer);
	}
}

void lexer_get_token(Lexer * lexer, Token * token) {
	lexer_skip(lexer);

	char curr = lexer_peek(lexer);
	
	token->line = lexer->line;
	
	// Parse HEX literal
	if (lexer_match(lexer, "0x")) {
		lexer->index += 2;
		curr = lexer_peek(lexer);

		unsigned long long hex = 0;

		while (isalnum(curr)) {
			hex <<= 4;
			if (curr >= 'A' && curr <= 'F') {
				hex += curr - 'A' + 10;
			} else if (curr >= 'a' && curr <= 'f') {
				hex += curr - 'a' + 10;
			} else if (isdigit(curr)) {
				hex += curr - '0';
			} else {
				abort(); // Invalid hex literal
			}

			curr = lexer_next(lexer);
		}
		
		token->type      = TOKEN_LITERAL_INT;
		token->sign      = 0;
		token->value_int = hex;

		return;
	}

	// Parse integer literal
	if (isdigit(curr) || ((curr == '+' || curr == '-') && isdigit(lexer->source[lexer->index + 1]))) {
		bool is_negative = false;

		if (curr == '+' || curr == '-') {
			is_negative = curr == '-';

			lexer_next(lexer);

			curr = lexer_peek(lexer);
		}

		unsigned long long value = 0;

		while (isdigit(curr)) {
			value *= 10;
			value += curr - '0';

			curr = lexer_next(lexer);
		}

		token->type      = TOKEN_LITERAL_INT;
		token->sign      = is_negative;
		token->value_int = value * (is_negative ? -1 : 1);

		return;
	}
	
	int match_length = 0;
	if (match_length = lexer_match(lexer, "true"))  { token->type = TOKEN_LITERAL_BOOL; token->value_int = 1; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "false")) { token->type = TOKEN_LITERAL_BOOL; token->value_int = 0; lexer->index += match_length; return; }

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
	} else if (curr == '\'') {
		char c = lexer_next(lexer);

		lexer_next(lexer); // Consume '

		token->type      = TOKEN_LITERAL_INT; // Treat as int literal
		token->sign      = false;
		token->value_int = c;

		lexer_next(lexer);

		return;
	}
	
	// Keywords
	if (match_length = lexer_match(lexer, "let"))      { token->type = TOKEN_KEYWORD_LET;      lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "cast"))     { token->type = TOKEN_KEYWORD_CAST;     lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "sizeof"))   { token->type = TOKEN_KEYWORD_SIZEOF;   lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "extern"))   { token->type = TOKEN_KEYWORD_EXTERN;   lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "if"))       { token->type = TOKEN_KEYWORD_IF;       lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "else"))     { token->type = TOKEN_KEYWORD_ELSE;     lexer->index += match_length; return; }
	//if (match_length = lexer_match(lexer, "for"))    { token->type = TOKEN_KEYWORD_FOR;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "while"))    { token->type = TOKEN_KEYWORD_WHILE;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "break"))    { token->type = TOKEN_KEYWORD_BREAK;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "continue")) { token->type = TOKEN_KEYWORD_CONTINUE; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "func"))     { token->type = TOKEN_KEYWORD_FUNC;     lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "return"))   { token->type = TOKEN_KEYWORD_RETURN;   lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "struct"))   { token->type = TOKEN_KEYWORD_STRUCT;   lexer->index += match_length; return; }

	// Bitshift Operators
	if (match_length = lexer_match(lexer, "<<")) { token->type = TOKEN_OPERATOR_SHIFT_LEFT;  lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, ">>")) { token->type = TOKEN_OPERATOR_SHIFT_RIGHT; lexer->index += match_length; return; }

	// Relational Operators
	if (match_length = lexer_match(lexer, "<=")) { token->type = TOKEN_OPERATOR_LT_EQ; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, ">=")) { token->type = TOKEN_OPERATOR_GT_EQ; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "<"))  { token->type = TOKEN_OPERATOR_LT;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, ">"))  { token->type = TOKEN_OPERATOR_GT;    lexer->index += match_length; return; }
	
	// Logical Operators
	if (match_length = lexer_match(lexer, "&&")) { token->type = TOKEN_OPERATOR_LOGICAL_AND; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "||")) { token->type = TOKEN_OPERATOR_LOGICAL_OR;  lexer->index += match_length; return; }
	
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
		case '.': token->type = TOKEN_DOT; lexer_next(lexer); return;

		case '(': token->type = TOKEN_PARENTESES_OPEN;  lexer_next(lexer); return;
		case ')': token->type = TOKEN_PARENTESES_CLOSE; lexer_next(lexer); return;

		case '{': token->type = TOKEN_BRACES_OPEN;  lexer_next(lexer); return;
		case '}': token->type = TOKEN_BRACES_CLOSE; lexer_next(lexer); return;

		case '+': token->type = TOKEN_OPERATOR_PLUS; 	 lexer_next(lexer); return;
		case '-': token->type = TOKEN_OPERATOR_MINUS; 	 lexer_next(lexer); return;
		case '*': token->type = TOKEN_OPERATOR_MULTIPLY; lexer_next(lexer); return; 
		case '/': token->type = TOKEN_OPERATOR_DIVIDE;   lexer_next(lexer); return;
		case '%': token->type = TOKEN_OPERATOR_MODULO;   lexer_next(lexer); return;

		case '!': token->type = TOKEN_OPERATOR_LOGICAL_NOT; lexer_next(lexer); return;

		case '&': token->type = TOKEN_OPERATOR_BITWISE_AND; lexer_next(lexer); return;
		case '|': token->type = TOKEN_OPERATOR_BITWISE_OR;  lexer_next(lexer); return;
		case '^': token->type = TOKEN_OPERATOR_BITWISE_XOR; lexer_next(lexer); return;
		case '~': token->type = TOKEN_OPERATOR_BITWISE_NOT; lexer_next(lexer); return;

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

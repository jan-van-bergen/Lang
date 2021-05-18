#include "Lexer.h"

#include <string.h>
#include <assert.h>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "Error.h"

void lexer_init(Lexer * lexer, char const * source) {
	lexer->source_len = strlen(source);
	lexer->source     = source;

	lexer->index = 0;
	lexer->line  = 1;

	lexer->tokens_len = 0;
	lexer->tokens_cap = 32;
	lexer->tokens = mem_alloc(lexer->tokens_cap * sizeof(Token));
}

void lexer_mem_free(Lexer * lexer) {
	mem_free(lexer->tokens);
}

static char lexer_peek(Lexer const * lexer) {
	assert(lexer->index >= 0 && lexer->index <= lexer->source_len);

	return lexer->source[lexer->index];
}

static char lexer_next(Lexer * lexer) {
	return lexer->source[++lexer->index];
}

static void lexer_advance(Lexer * lexer, char target) {
	char c = lexer_next(lexer);
	if (c != target) {
		printf("ERROR: Lexer encountered unexpeced char!\n");
		printf("Expected: '%c'\n", target);
		printf("Observed: '%c'\n", c);
		error(ERROR_LEXER);
	}
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

static bool lexer_reached_end(Lexer const * lexer) {
	lexer_skip(lexer);

	return lexer_peek(lexer) == '\0' || lexer->index == lexer->source_len;
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
				printf("ERROR: Invalid character '%c' in hex literal!\n", curr);
				error(ERROR_LEXER);
			}

			curr = lexer_next(lexer);
		}
		
		token->type      = TOKEN_LITERAL_INT;
		token->sign      = 0;
		token->value_int = hex;

		return;
	}

	// Parse integer or float/double literal
	if (isdigit(curr) || ((curr == '+' || curr == '-') && isdigit(lexer->source[lexer->index + 1]))) {
		int index_lit_start = lexer->index;
		
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

		// If we encounter a dot parse as float/double literal
		if (curr == '.') {
			curr = lexer_next(lexer);

			while (isdigit(curr)) curr = lexer_next(lexer);

			if (curr == 'f') {
				lexer_next(lexer);

				token->type        = TOKEN_LITERAL_F32;
				token->value_float = strtof(lexer->source + index_lit_start, NULL);
			} else {
				token->type         = TOKEN_LITERAL_F64;
				token->value_double = atof(lexer->source + index_lit_start);
			}
		} else {
			token->type      = TOKEN_LITERAL_INT;
			token->sign      = is_negative;
			token->value_int = value * (is_negative ? -1 : 1);
		}

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
		char * str     = mem_alloc(str_len + 1);

		memcpy(str, lexer->source + index_str_start, index_str_end - index_str_start);
		str[str_len] = '\0';

		token->type      = TOKEN_LITERAL_STRING;
		token->value_str = str;

		lexer_next(lexer);

		return;
	} else if (curr == '\'') {
		char c = lexer_next(lexer);

		if (c == '\\') {
			c = lexer_next(lexer);

			switch (c) {
				case '0': c = '\0'; break;
				case 'r': c = '\r'; break;
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;

				default: error(ERROR_LEXER);
			}
		}

		lexer_advance(lexer, '\'');

		token->type      = TOKEN_LITERAL_CHAR;
		token->sign      = false;
		token->value_int = c;

		lexer_next(lexer);

		return;
	}
	
	// Aassignment Operators (excluding = see switch statement)
	if (match_length = lexer_match(lexer, "+="))  { token->type = TOKEN_ASSIGN_PLUS;        lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "-="))  { token->type = TOKEN_ASSIGN_MINUS;       lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "*="))  { token->type = TOKEN_ASSIGN_MULTIPLY;    lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "/="))  { token->type = TOKEN_ASSIGN_DIVIDE;      lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "%="))  { token->type = TOKEN_ASSIGN_MODULO;      lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "<<=")) { token->type = TOKEN_ASSIGN_SHIFT_LEFT;  lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, ">>=")) { token->type = TOKEN_ASSIGN_SHIFT_RIGHT; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "&="))  { token->type = TOKEN_ASSIGN_BITWISE_AND; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "^="))  { token->type = TOKEN_ASSIGN_BITWISE_XOR; lexer->index += match_length; return; }
	if (match_length = lexer_match(lexer, "|="))  { token->type = TOKEN_ASSIGN_BITWISE_OR;  lexer->index += match_length; return; }

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

	if (match_length = lexer_match(lexer, "->")) { token->type = TOKEN_ARROW; lexer->index += match_length; return; }

	switch (curr) {
		case '.': token->type = TOKEN_DOT; lexer_next(lexer); return;

		case '(': token->type = TOKEN_PARENTESES_OPEN;  lexer_next(lexer); return;
		case ')': token->type = TOKEN_PARENTESES_CLOSE; lexer_next(lexer); return;

		case '{': token->type = TOKEN_BRACES_OPEN;  lexer_next(lexer); return;
		case '}': token->type = TOKEN_BRACES_CLOSE; lexer_next(lexer); return;

		case '[': token->type = TOKEN_SQUARE_BRACES_OPEN;  lexer_next(lexer); return;
		case ']': token->type = TOKEN_SQUARE_BRACES_CLOSE; lexer_next(lexer); return;

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

	// Token will be identifier or keyword from now on
	int index_str_start = lexer->index;
		
	curr = lexer_next(lexer); // Consume first letter of identifier

	while (isalnum(curr) || curr == '_') curr = lexer_next(lexer);
			
	int index_str_end = lexer->index;

	int    str_len = index_str_end - index_str_start;
	char * str     = mem_alloc(str_len + 1);

	memcpy(str, lexer->source + index_str_start, index_str_end - index_str_start);
	str[str_len] = '\0';

	// Keywords
	if (strcmp(str, "cast")     == 0) { token->type = TOKEN_KEYWORD_CAST;     return; }
	if (strcmp(str, "sizeof")   == 0) { token->type = TOKEN_KEYWORD_SIZEOF;   return; }
	if (strcmp(str, "null")     == 0) { token->type = TOKEN_KEYWORD_NULL;     return; }
	if (strcmp(str, "extern")   == 0) { token->type = TOKEN_KEYWORD_EXTERN;   return; }
	if (strcmp(str, "export")   == 0) { token->type = TOKEN_KEYWORD_EXPORT;   return; }
	if (strcmp(str, "if")       == 0) { token->type = TOKEN_KEYWORD_IF;       return; }
	if (strcmp(str, "else")     == 0) { token->type = TOKEN_KEYWORD_ELSE;     return; }
	if (strcmp(str, "while")    == 0) { token->type = TOKEN_KEYWORD_WHILE;    return; }
	if (strcmp(str, "break")    == 0) { token->type = TOKEN_KEYWORD_BREAK;    return; }
	if (strcmp(str, "continue") == 0) { token->type = TOKEN_KEYWORD_CONTINUE; return; }
	if (strcmp(str, "func")     == 0) { token->type = TOKEN_KEYWORD_FUNC;     return; }
	if (strcmp(str, "return")   == 0) { token->type = TOKEN_KEYWORD_RETURN;   return; }
	if (strcmp(str, "struct")   == 0) { token->type = TOKEN_KEYWORD_STRUCT;   return; }

	token->type = TOKEN_IDENTIFIER;
	token->value_str = str;
}

static Token * lexer_new_token(Lexer * lexer) {
	if (lexer->tokens_len == lexer->tokens_cap) {
		lexer->tokens_cap *= 2;
		lexer->tokens = mem_realloc(lexer->tokens, lexer->tokens_cap * sizeof(Token));
	}
		
	return lexer->tokens + lexer->tokens_len++;
}

void lexer_lex(Lexer * lexer) {
	while (!lexer_reached_end(lexer)) {	
		lexer_get_token(lexer, lexer_new_token(lexer));
	}

	// Append EOF Token
	lexer_new_token(lexer)->type = TOKEN_EOF;
}

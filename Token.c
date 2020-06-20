#include "Token.h"

void token_to_string(Token const * token, char * string, int string_size) {
	switch (token->type) {
		case TOKEN_IDENTIFIER:     sprintf_s(string, string_size, "%s",     token->value_str);                     return;
		case TOKEN_LITERAL_INT:    sprintf_s(string, string_size, "%i",     token->value_int);                     return;
		case TOKEN_LITERAL_BOOL:   sprintf_s(string, string_size, "%s",     token->value_char ? "true" : "false"); return;
		case TOKEN_LITERAL_STRING: sprintf_s(string, string_size, "\"%s\"", token->value_str);                     return;

		case TOKEN_KEYWORD_LET:    strcpy_s(string, string_size, "let");    return;
		case TOKEN_KEYWORD_IF:	   strcpy_s(string, string_size, "if");     return;
		case TOKEN_KEYWORD_ELSE:   strcpy_s(string, string_size, "else");   return;
		//case TOKEN_KEYWORD_FOR:	   strcpy_s(string, string_size, "for");    return;
		case TOKEN_KEYWORD_WHILE:  strcpy_s(string, string_size, "while");  return;
		case TOKEN_KEYWORD_FUNC:   strcpy_s(string, string_size, "func");   return;
		//case TOKEN_KEYWORD_STRUCT: strcpy_s(string, string_size, "struct"); return;
		
		case TOKEN_PARENTESES_OPEN:  strcpy_s(string, string_size, "("); return;
		case TOKEN_PARENTESES_CLOSE: strcpy_s(string, string_size, ")"); return;

		case TOKEN_BRACES_OPEN:  strcpy_s(string, string_size, "{"); return;
		case TOKEN_BRACES_CLOSE: strcpy_s(string, string_size, "}"); return;
		
		case TOKEN_ARROW: strcpy_s(string, string_size, "->"); return;

		case TOKEN_ASSIGN:		    strcpy_s(string, string_size, "=");  return;
		case TOKEN_ASSIGN_PLUS:     strcpy_s(string, string_size, "+="); return;
		case TOKEN_ASSIGN_MINUS:    strcpy_s(string, string_size, "-="); return;
		case TOKEN_ASSIGN_MULTIPLY: strcpy_s(string, string_size, "*="); return;
		case TOKEN_ASSIGN_DIVIDE:   strcpy_s(string, string_size, "/="); return;
		
		case TOKEN_OPERATOR_PLUS:	  strcpy_s(string, string_size, "+"); return;
		case TOKEN_OPERATOR_MINUS:	  strcpy_s(string, string_size, "-"); return;
		case TOKEN_OPERATOR_MULTIPLY: strcpy_s(string, string_size, "*"); return;
		case TOKEN_OPERATOR_DIVIDE:   strcpy_s(string, string_size, "/"); return;

		case TOKEN_OPERATOR_SHIFT_LEFT:  strcpy_s(string, string_size, "<<"); return;
		case TOKEN_OPERATOR_SHIFT_RIGHT: strcpy_s(string, string_size, ">>"); return;

		case TOKEN_OPERATOR_LT:    strcpy_s(string, string_size, "<");  return;
		case TOKEN_OPERATOR_GT:    strcpy_s(string, string_size, ">");  return;
		case TOKEN_OPERATOR_LT_EQ: strcpy_s(string, string_size, "<="); return;
		case TOKEN_OPERATOR_GT_EQ: strcpy_s(string, string_size, ">="); return; 

		case TOKEN_OPERATOR_EQ:	strcpy_s(string, string_size, "=="); return;
		case TOKEN_OPERATOR_NE: strcpy_s(string, string_size, "!="); return;

		case TOKEN_OPERATOR_INC:  strcpy_s(string, string_size, "++"); return;  
		case TOKEN_OPERATOR_DEC:  strcpy_s(string, string_size, "--"); return;  

		case TOKEN_COMMA:	  strcpy_s(string, string_size, ","); return;
		case TOKEN_COLON:	  strcpy_s(string, string_size, ":"); return;
		case TOKEN_SEMICOLON: strcpy_s(string, string_size, ";"); return;

		case TOKEN_EOF: strcpy_s(string, string_size, "EOF"); return;

		default: strcpy_s(string, string_size, "Unknown Token"); return;
	}
}

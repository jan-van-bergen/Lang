#include "Token.h"

#include <string.h>
#include <stdio.h>

#include "Util.h"

void token_to_string(Token const * token, char * string, int string_size) {
	switch (token->type) {
		case TOKEN_IDENTIFIER: sprintf_s(string, string_size, "%s", token->value_str); return;

		case TOKEN_LITERAL_INT: {
			if (token->sign) {
				sprintf_s(string, string_size, "%lld", token->value_int);
			} else {
				sprintf_s(string, string_size, "%llu", token->value_int);
			}

			return;
		}

		case TOKEN_LITERAL_F32: sprintf_s(string, string_size, "%ff", token->value_float);  break;
		case TOKEN_LITERAL_F64: sprintf_s(string, string_size, "%f",  token->value_double); break;

		case TOKEN_LITERAL_BOOL:   sprintf_s(string, string_size, "%s",     token->value_int ? "true" : "false"); return;
		case TOKEN_LITERAL_STRING: sprintf_s(string, string_size, "\"%s\"", token->value_str);                    return;

		case TOKEN_KEYWORD_LET:      strcpy_s(string, string_size, "let");      return;
		case TOKEN_KEYWORD_CAST:     strcpy_s(string, string_size, "cast");     return;
		case TOKEN_KEYWORD_SIZEOF:   strcpy_s(string, string_size, "sizeof");   return;
		case TOKEN_KEYWORD_EXTERN:   strcpy_s(string, string_size, "extern");   return;
		case TOKEN_KEYWORD_IF:	     strcpy_s(string, string_size, "if");       return;
		case TOKEN_KEYWORD_ELSE:     strcpy_s(string, string_size, "else");     return;
		//case TOKEN_KEYWORD_FOR:	   strcpy_s(string, string_size, "for");    return;
		case TOKEN_KEYWORD_WHILE:    strcpy_s(string, string_size, "while");    return;
		case TOKEN_KEYWORD_BREAK:    strcpy_s(string, string_size, "break");    return;
		case TOKEN_KEYWORD_CONTINUE: strcpy_s(string, string_size, "continue"); return;
		case TOKEN_KEYWORD_FUNC:     strcpy_s(string, string_size, "func");     return;
		case TOKEN_KEYWORD_RETURN:   strcpy_s(string, string_size, "return");   return;
		case TOKEN_KEYWORD_STRUCT:   strcpy_s(string, string_size, "struct");   return;
		
		case TOKEN_PARENTESES_OPEN:  strcpy_s(string, string_size, "("); return;
		case TOKEN_PARENTESES_CLOSE: strcpy_s(string, string_size, ")"); return;

		case TOKEN_BRACES_OPEN:  strcpy_s(string, string_size, "{"); return;
		case TOKEN_BRACES_CLOSE: strcpy_s(string, string_size, "}"); return;
		
		case TOKEN_SQUARE_BRACES_OPEN:  strcpy_s(string, string_size, "["); return;
		case TOKEN_SQUARE_BRACES_CLOSE: strcpy_s(string, string_size, "]"); return;

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
		case TOKEN_OPERATOR_MODULO:   strcpy_s(string, string_size, "%"); return;
			
		case TOKEN_OPERATOR_LT_EQ: strcpy_s(string, string_size, "<="); return;
		case TOKEN_OPERATOR_GT_EQ: strcpy_s(string, string_size, ">="); return; 
		case TOKEN_OPERATOR_LT:    strcpy_s(string, string_size, "<");  return;
		case TOKEN_OPERATOR_GT:    strcpy_s(string, string_size, ">");  return;

		case TOKEN_OPERATOR_EQ:	strcpy_s(string, string_size, "=="); return;
		case TOKEN_OPERATOR_NE: strcpy_s(string, string_size, "!="); return;

		case TOKEN_OPERATOR_SHIFT_LEFT:  strcpy_s(string, string_size, "<<"); return;
		case TOKEN_OPERATOR_SHIFT_RIGHT: strcpy_s(string, string_size, ">>"); return;

		case TOKEN_OPERATOR_LOGICAL_AND: strcpy_s(string, string_size, "&&"); return;
		case TOKEN_OPERATOR_LOGICAL_OR:	 strcpy_s(string, string_size, "||"); return;
		case TOKEN_OPERATOR_LOGICAL_NOT: strcpy_s(string, string_size, "!");  return;
			
		case TOKEN_OPERATOR_BITWISE_AND: strcpy_s(string, string_size, "&"); return;
		case TOKEN_OPERATOR_BITWISE_OR:  strcpy_s(string, string_size, "|"); return;
		case TOKEN_OPERATOR_BITWISE_XOR: strcpy_s(string, string_size, "^"); return;
		case TOKEN_OPERATOR_BITWISE_NOT: strcpy_s(string, string_size, "~"); return;

		case TOKEN_OPERATOR_INC: strcpy_s(string, string_size, "++"); return;  
		case TOKEN_OPERATOR_DEC: strcpy_s(string, string_size, "--"); return;  

		case TOKEN_COMMA:	  strcpy_s(string, string_size, ","); return;
		case TOKEN_COLON:	  strcpy_s(string, string_size, ":"); return;
		case TOKEN_SEMICOLON: strcpy_s(string, string_size, ";"); return;

		case TOKEN_EOF: strcpy_s(string, string_size, "EOF"); return;

		default: strcpy_s(string, string_size, "Unknown Token"); return;
	}
}

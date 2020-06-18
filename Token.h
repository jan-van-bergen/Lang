#pragma once

typedef struct Token {
	enum Type {
		TOKEN_IDENTIFIER,
		TOKEN_STRING_LITERAL,
		
		TOKEN_PARENTESES_OPEN,
		TOKEN_PARENTESES_CLOSE,

		TOKEN_COLON,
		TOKEN_EQUALS,

		TOKEN_SEMICOLON
	} type;

	const char * value;
} Token;

void token_init(Token * token, Token::Type type, const char * value);

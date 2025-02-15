#include <stdio.h>
#include <stdlib.h>

typedef enum{
	TOKEN_OBRACE,
	TOKEN_CBRACE,
	TOKEN_OPAREN,
	TOKEN_CPAREN,
	TOKEN_SEMICOL,
	TOKEN_IDENTIFIER,
	KEYW_RETURN,
	KEYW_INT,
	LITERAL_INT,
	TOKEN_EOF,
	TOKEN_INVALID,
} TokenType;

typedef struct{
	TokenType type;
	char* value;
} Token;

typedef struct{
	size_t pos;
	const char *input;
} Lexer;

Lexer* create_lexer(const char* input);
Token* create_token(TokenType type, const char* value);
void free_token(Token* token);

#include <stdio.h>
#include <stdlib.h>
#ifndef LEXER_H
#define LEXER_H
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
	OP_NEGATION,
	OP_COMPL,
	OP_NEGATIONL,
	OP_ADD,
	OP_MUL,
	OP_DIV,
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

Lexer* createLexer(const char* input);
Token* createToken(TokenType type, char* value);
void freeToken(Token* token);
Token** lex(const char* filename, int* token_count);
#endif

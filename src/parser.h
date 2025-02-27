#include <stdio.h>
#include "lexer.h"
#ifndef PARSER_H
#define PARSER_H
typedef enum {
	AST_PROGRAM,
	AST_FUNCTION,
	AST_BLOCK,
	AST_STATEMENT,
	AST_EXPRESSION,
	AST_CONSTANT
} ASTNodeType;

typedef struct ASTNode {
	ASTNodeType type;
	union {
		// AST_PROGRAM: list of functions
		struct {
			struct ASTNode **functions;
			int functionCount;
		} program;

		// AST_FUNCTION: function name and block
		struct {
			char* name;
			struct ASTNode *body;
		} function;

		// AST_BLOCK: list of statements
		struct {
			struct ASTNode **statements;
			int statementCount;
		} block;

		// AST_STATEMENT: simple expression
		struct {
			struct ASTNode *expression;
		} statement;

		// AST_EXPRESSION: contains a constant for now
		struct {
			struct ASTNode *constant;
		} expression;

		// AST_CONSTANT: numeric constant (as string)
		struct {
			char *value;
		} constant;
	};
} ASTNode;

typedef struct {
    Token **tokens;    // Array of Token pointers
	int currentIndex;  // Current index into the tokens array
	int tokenCount;    // Total number of tokens in the array
	int errorFlag;     // Flag for error state
} Parser;

ASTNode *newASTNode(ASTNodeType type);
ASTNode *parseProgram(Parser* parser);
ASTNode *parseFunction(Parser* parser);
ASTNode *parseBlock(Parser* parser);
ASTNode *parseStatement(Parser* parser);
ASTNode *parseExpression(Parser* parser);
ASTNode *parseConstant(Parser* parser);
void consume(Parser* parser, TokenType expected, const char *errorMsg);
Token *currentToken(Parser *parser);
#endif

#include "lexer.h"
#ifndef PARSER_H
#define PARSER_H
typedef enum {
	AST_PROGRAM,
	AST_FUNCTION,
	AST_BLOCK,
	AST_STATEMENT,
	AST_DECL,
	AST_EXPRESSION,
	AST_CONSTANT,
	AST_UNARY,
	AST_BINARY,
	AST_IDENTIFIER,
	AST_FACTOR
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
			struct ASTNode *declaration;
		} statement;

		// AST_DECL: declaration 
		struct {
			char *type;
			struct ASTNode *identifier;
			struct ASTNode *initializer;
		} decl;

		// AST_EXPRESSION: contains two more sub-expressions
		struct {
			struct ASTNode *term;
			struct ASTNode *binary;
		} expression;

		// AST_FACTOR: contains a sub-expression, an operator, a factor, or a constant
		struct {
			struct ASTNode *expression;
			struct ASTNode *factor;
			struct ASTNode *unary;
			struct ASTNode *constant;
			struct ASTNode *identifier;
		} factor;
		// AST_CONSTANT: numeric constant (as string)
		struct {
			char *value;
		} constant;

		// AST_UNARY: unary symbol (as string)
		struct {
			char *value;
		} unary;

		// AST_IDENTIFIER: identifier 
		struct {
			char *value;
		} identifier;

		// AST_BINARY: binary operator (as string)
		struct {
			struct ASTNode *left;
			char *value;
			struct ASTNode *right;
		} binary;

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
ASTNode *parseDeclaration(Parser* parser);
ASTNode *parseExpression(Parser* parser, int precedence);
ASTNode *parseUnary(Parser* parser);
ASTNode *parseBinary(Parser* parser);
ASTNode *parseFactor(Parser* parser);
ASTNode *parseConstant(Parser* parser);
ASTNode *parseIdentifier(Parser *parser);
int precedence(TokenType type);
void consume(Parser* parser, TokenType expected, const char *errorMsg);
Token *currentToken(Parser *parser);
Token *nextToken(Parser *parser);
void printAST(ASTNode *node, int indent);
void freeAST(ASTNode *node);
void freeTokens(Token **tokens, int tokenCount);
#endif

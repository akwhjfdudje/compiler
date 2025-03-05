#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {

	if (argc < 2) {
  		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}

    int token_count = 0;
    Token** tokens = lex(argv[1], &token_count);
    if (!tokens) {
		return 1; // Error occurred in lex()
	}

	printf("%d\n", token_count);
    for (int i = 0; i < token_count && tokens[i] != NULL; i++) {
		printf("Token: Type=%d, Value='%s'\n", tokens[i]->type, tokens[i]->value);
		//freeToken(tokens[i]); // Free each token
	}
	//return 0;
    // Initialize the parser context.
	Parser parser;
	parser.tokens = tokens;
	parser.currentIndex = 0;
	parser.tokenCount = token_count;
	parser.errorFlag = 0;

	// Parse the tokens into an AST.
	ASTNode *ast = parseProgram(&parser);
	printf("Parsing complete!\n\nAST:\n");
	printAST(ast, 0);

	freeAST(ast);
	freeTokens(tokens, token_count);

	return 0;
}


/*
int main() {
    
	//For demonstration purposes, we build an array of tokens manually.
	//In a real application, a lexer would generate this array from source code.
	
    int tokenCount = 9;
    Token **tokens = malloc(sizeof(Token *) * tokenCount);
    
    tokens[0] = malloc(sizeof(Token)); tokens[0]->type = KEYW_INT;         tokens[0]->value = strdup("int");
	tokens[1] = malloc(sizeof(Token)); tokens[1]->type = TOKEN_IDENTIFIER; tokens[1]->value = strdup("main");
	tokens[2] = malloc(sizeof(Token)); tokens[2]->type = TOKEN_OPAREN;     tokens[2]->value = strdup("(");
	tokens[3] = malloc(sizeof(Token)); tokens[3]->type = TOKEN_CPAREN;     tokens[3]->value = strdup(")");
	tokens[4] = malloc(sizeof(Token)); tokens[4]->type = TOKEN_OBRACE;     tokens[4]->value = strdup("{");
	tokens[5] = malloc(sizeof(Token)); tokens[5]->type = KEYW_RETURN;      tokens[5]->value = strdup("return");
	tokens[6] = malloc(sizeof(Token)); tokens[6]->type = LITERAL_INT;      tokens[6]->value = strdup("42");
	tokens[7] = malloc(sizeof(Token)); tokens[7]->type = TOKEN_SEMICOL;    tokens[7]->value = strdup(";");
	tokens[8] = malloc(sizeof(Token)); tokens[8]->type = TOKEN_CBRACE;     tokens[8]->value = strdup("}");
    
	printf("%d\n", tokenCount);
    for (int i = 0; i < tokenCount && tokens[i] != NULL; i++) {
		printf("Token: Type=%d, Value='%s'\n", tokens[i]->type, tokens[i]->value);
		//freeToken(tokens[i]); // Free each token
	}
    // Initialize the parser context.
    Parser parser;
    parser.tokens = tokens;
    parser.currentIndex = 0;
    parser.tokenCount = tokenCount;
    parser.errorFlag = 0;
    
    // Parse the tokens into an AST.
    ASTNode *ast = parseProgram(&parser);
    printf("Parsing complete!\n\nAST:\n");
    printAST(ast, 0);
    
    // Clean up allocated memory.
    freeAST(ast);
    freeTokens(tokens, tokenCount);
    
    return 0;
}
*/

void reportError(Parser *parser, const char *message) {
    fprintf(stderr, "Parse error: %s at token '%s'\n",
		message,
		currentToken(parser)->value ? currentToken(parser)->value : "EOF");
    parser->errorFlag = 1;
}

// Get tokens, report an error
void consume(Parser *parser, TokenType expected, const char *errorMsg) {
	if (currentToken(parser)->type != expected) {
		reportError(parser, errorMsg);
		return;
	}
    if (currentToken(parser)->type) free(currentToken(parser)->value);
    parser->currentIndex++;
}

Token *currentToken(Parser *parser) {
    if (parser->currentIndex < parser->tokenCount) {
		return parser->tokens[parser->currentIndex];
	}
	static Token eofToken = { TOKEN_EOF, NULL };
	return &eofToken;
}

ASTNode *newASTNode(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
	node->type = type;
	return node;
}

void skip(Parser *parser) {
	while (currentToken(parser)->type != TOKEN_SEMICOL &&
			currentToken(parser)->type != TOKEN_CBRACE &&
			currentToken(parser)->type != TOKEN_EOF) {
		parser->currentIndex++;
	}
	if (currentToken(parser)->type == TOKEN_SEMICOL) {
		parser->currentIndex++;
	}
	parser->errorFlag = 0;
}

ASTNode *parseProgram(Parser* parser) {
	ASTNode *node = newASTNode(AST_PROGRAM);
	node->program.functions = NULL;
	node->program.functionCount = 0;

	while (currentToken(parser)->type != TOKEN_EOF) {
        // In top-level, we only expect functions starting with 'int'.
		if (currentToken(parser)->type != KEYW_INT) {
			reportError(parser, "Expected function to start with 'int'");
			// Skip the current token to avoid an infinite loop.
			parser->currentIndex++;
			continue;
		}
		ASTNode *func = parseFunction(parser);
		if (func) {
			node->program.functionCount++;
			node->program.functions = realloc(node->program.functions,
			sizeof(ASTNode *) * node->program.functionCount);
			node->program.functions[node->program.functionCount - 1] = func;
		} else {
			skip(parser);
		}
	}
	return node;
}

ASTNode *parseFunction(Parser* parser) {
	if (currentToken(parser)->type != KEYW_INT) {
		reportError(parser, "Function must start with 'int'");
		skip(parser);
		return NULL;
	}
	consume(parser, KEYW_INT, "Expected 'int'");

	if (currentToken(parser)->type != TOKEN_IDENTIFIER) {
		reportError(parser, "Expected function name identifier");
		skip(parser);
		return NULL;
	}
	char *funcName = strdup(currentToken(parser)->value);
	consume(parser, TOKEN_IDENTIFIER, "After function name");

	consume(parser, TOKEN_OPAREN, "Expected '(' after function name");
	consume(parser, TOKEN_CPAREN, "Expected ')' after '('");

	ASTNode *body = parseBlock(parser);
	if (!body) {
		reportError(parser, "Invalid function body");
		return NULL;
	}

	ASTNode *node = newASTNode(AST_FUNCTION);
	node->function.name = funcName;
	node->function.body = body;
	return node;
}

ASTNode *parseBlock(Parser* parser) {
	if (currentToken(parser)->type != TOKEN_OBRACE) {
		reportError(parser, "Expected '{' to start block");
		skip(parser);
		return NULL;
	}
	consume(parser, TOKEN_OBRACE, "Start block");

	ASTNode *node = newASTNode(AST_BLOCK);
	node->block.statements = NULL;
	node->block.statementCount = 0;

	while (currentToken(parser)->type != TOKEN_CBRACE &&
		currentToken(parser)->type != TOKEN_EOF) {
		ASTNode *stmt = parseStatement(parser);
		if (stmt) {
			node->block.statementCount++;
			node->block.statements = realloc(node->block.statements, sizeof(ASTNode *) * node->block.statementCount);
			node->block.statements[node->block.statementCount - 1] = stmt;
		} else {
			skip(parser);
		}
	}

	if (currentToken(parser)->type == TOKEN_CBRACE) {
		consume(parser, TOKEN_CBRACE, "End block");
	} else {
		reportError(parser, "Expected '}' to end block");
	}
	return node;
}

ASTNode *parseStatement(Parser* parser) {
	if (currentToken(parser)->type != KEYW_RETURN) {
		reportError(parser, "Expected return keyword in statement");
		skip(parser);
		return NULL;
	}
	consume(parser, KEYW_RETURN, "Return statement start");
	ASTNode *node = newASTNode(AST_STATEMENT);
	node->statement.expression = parseExpression(parser);
	if (parser->errorFlag) {
		skip(parser);
		return NULL;
	}
	if (currentToken(parser)->type != TOKEN_SEMICOL) {
		reportError(parser, "Expected ';' after expression");
		skip(parser);
		return NULL;
	}
	consume(parser, TOKEN_SEMICOL, "After expression");
	return node;
}

ASTNode *parseExpression(Parser* parser) {
    ASTNode *node = newASTNode(AST_EXPRESSION);
	node->expression.constant = parseConstant(parser);
	return node;
}

ASTNode *parseConstant(Parser* parser) {
	if (currentToken(parser)->type != LITERAL_INT) {
		reportError(parser, "Expected constant (number) in expression");
		skip(parser);
		return NULL;
	}
	ASTNode *node = newASTNode(AST_CONSTANT);
	node->constant.value = strdup(currentToken(parser)->value);
	consume(parser, LITERAL_INT, "After constant");
	return node;
}

/* --- AST Printing for Debug --- */
void printAST(ASTNode *node, int indent) {
	for (int i = 0; i < indent; i++) printf("  ");
	switch (node->type) {
		case AST_PROGRAM:
			printf("Program:\n");
			for (int i = 0; i < node->program.functionCount; i++) {
				printAST(node->program.functions[i], indent + 1);
			}
			break;
		case AST_FUNCTION:
			printf("Function: %s\n", node->function.name);
			printAST(node->function.body, indent + 1);
			break;
		case AST_BLOCK:
			printf("Block:\n");
			for (int i = 0; i < node->block.statementCount; i++) {
				printAST(node->block.statements[i], indent + 1);
			}
			break;
		case AST_STATEMENT:
			printf("Statement:\n");
			printAST(node->statement.expression, indent + 1);
			break;
		case AST_EXPRESSION:
			printf("Expression:\n");
			printAST(node->expression.constant, indent + 1);
			break;
		case AST_CONSTANT:
			printf("Constant: %s\n", node->constant.value);
			break;
		default:
			printf("Unknown AST Node\n");
			break;
		}
}

// Recursively free the AST nodes.
void freeAST(ASTNode *node) {
	if (!node)
		return;
	switch (node->type) {
		case AST_PROGRAM:
			for (int i = 0; i < node->program.functionCount; i++) {
				freeAST(node->program.functions[i]);
			}
			free(node->program.functions);
			break;
		case AST_FUNCTION:
			free(node->function.name);
			freeAST(node->function.body);
			break;
		case AST_BLOCK:
			for (int i = 0; i < node->block.statementCount; i++) {
				freeAST(node->block.statements[i]);
			}
			free(node->block.statements);
			break;
		case AST_STATEMENT:
			freeAST(node->statement.expression);
			break;
		case AST_EXPRESSION:
			freeAST(node->expression.constant);
			break;
		case AST_CONSTANT:
			free(node->constant.value);
			break;
		default:
			break;
	}
	free(node);
}

// Free the array of tokens.
void freeTokens(Token **tokens, int tokenCount) {
	for (int i = 0; i < tokenCount; i++) {
		if (tokens[i]->value) free(tokens[i]->value);
		free(tokens[i]);
	}
	free(tokens);
}



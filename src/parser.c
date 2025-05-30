#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>


void reportError(Parser *parser, const char *message) {
    fprintf(stderr, "Parse error: %s at token '%s', on line %d\n",
        message,
        currentToken(parser)->value ? currentToken(parser)->value : "EOF",
        currentToken(parser)->line);
    parser->errorFlag = 1;
}

int precedence(TokenType type) {
    switch (type) {
        case OP_MUL:
        case OP_DIV:        return 8;
        case OP_ADD:
        case OP_NEGATION:   return 7;
        case OP_LESS:
        case OP_LESSEQ:
        case OP_GREATER:
        case OP_GREATEREQ:  return 6;
        case OP_EQ:
        case OP_NOTEQ:      return 5;
        case OP_AND:        return 4;
        case OP_OR:         return 3;
        case OP_Q:          return 2;
        case OP_ASSN:       return 1;
        default:            return 0;
    }
}

// Get tokens, report an error
void consume(Parser *parser, TokenType expected, const char *errorMsg) {
    if (currentToken(parser)->type != expected) {
        reportError(parser, errorMsg);
        return;
    }
    parser->currentIndex++;
}

Token *currentToken(Parser *parser) {
    if (parser->currentIndex < parser->tokenCount) {
        return parser->tokens[parser->currentIndex];
    }
    static Token eofToken = { TOKEN_EOF, NULL };
    return &eofToken;
}

Token *nextToken(Parser *parser) {
    if (parser->currentIndex + 1 < parser->tokenCount) {
        return parser->tokens[parser->currentIndex + 1];
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
    //parser->errorFlag = 0;
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
    // TODO: implement: https://stackoverflow.com/questions/22419790/c-error-expected-expression-before-int#22420796
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
    if (currentToken(parser)->type == KEYW_RETURN) {
        ASTNode *node = newASTNode(AST_STATEMENT);
        node->statement.retn = parseReturn(parser);
        node->statement.declaration = NULL;
        node->statement.expression = NULL;
        node->statement.ifstatement = NULL;
        node->statement.compound = NULL;
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
    if (currentToken(parser)->type == KEYW_INT) {
        ASTNode *node = newASTNode(AST_STATEMENT);
        node->statement.retn = NULL;
        node->statement.declaration = parseDeclaration(parser);
        node->statement.expression = NULL;
        node->statement.ifstatement = NULL;
        node->statement.compound = NULL;
        return node;
    }
    if (currentToken(parser)->type == KEYW_IF
        || currentToken(parser)->type == KEYW_ELSE) {
        ASTNode *node = newASTNode(AST_STATEMENT);
        node->statement.retn = NULL;
        node->statement.declaration = NULL;
        node->statement.expression = NULL;
        node->statement.ifstatement = parseIf(parser);
        node->statement.compound = NULL;
        return node;
    }
    if (currentToken(parser)->type == TOKEN_OBRACE) {
        ASTNode *node = newASTNode(AST_STATEMENT);
        node->statement.retn = NULL;
        node->statement.declaration = NULL;
        node->statement.expression = NULL;
        node->statement.ifstatement = NULL;
        node->statement.compound = parseBlock(parser);
        return node;
    }
    ASTNode *node = newASTNode(AST_STATEMENT);
    node->statement.retn = NULL;
    node->statement.declaration = NULL;
    node->statement.expression = parseExpression(parser, 0);
    if (node->statement.expression == NULL) {
        reportError(parser, "Invalid expression");
        skip(parser);
        return NULL;
    }
    if (currentToken(parser)->type != TOKEN_SEMICOL) {
        reportError(parser, "Expected ';' at end of statement");
        skip(parser);
        return NULL;
    }
    consume(parser, TOKEN_SEMICOL, "After expression");
    return node;
}

ASTNode *parseIf(Parser* parser) {
    if (currentToken(parser)->type != KEYW_IF) {
        reportError(parser, "Expected if");
        skip(parser);
        return NULL;
    }

    consume(parser, KEYW_IF, "Start of conditional");
    consume(parser, TOKEN_OPAREN, "Expected parenthesis");
    ASTNode* node = newASTNode(AST_IF);
    node->ifstmt.condition = parseExpression(parser, 0);
    consume(parser, TOKEN_CPAREN, "Expected end of expression.");
    node->ifstmt.body = currentToken(parser)->type == TOKEN_OBRACE ? 
                            parseBlock(parser) : parseStatement(parser);
    if (currentToken(parser)->type == KEYW_ELSE) {
        consume(parser, KEYW_ELSE, "Start of else statement");
        node->ifstmt.elsestmt = currentToken(parser)->type == TOKEN_OBRACE ? 
                                parseBlock(parser) : parseStatement(parser);
    }
    return node;
}

ASTNode *parseReturn(Parser* parser) {
    consume(parser, KEYW_RETURN, "Return statement start");
    ASTNode* node = newASTNode(AST_RETURN);
    node->retn.expression = parseExpression(parser, 0);
    return node;
}

ASTNode *parseDeclaration(Parser* parser) {
    if (currentToken(parser)->type != KEYW_INT) {
        reportError(parser, "Expected int keyword in declaration");
        skip(parser);
        return NULL;
    }
    consume(parser, KEYW_INT, "Declaration start");
    ASTNode *node = newASTNode(AST_DECL);
    node->decl.identifier = parseIdentifier(parser);
    if (currentToken(parser)->type == OP_ASSN) {
        consume(parser, OP_ASSN, "Start of assignment");
        node->decl.initializer = parseExpression(parser, 0);
        node->decl.type = "int";
        consume(parser, TOKEN_SEMICOL, "End of declaration");
        return node;
    }
    if (currentToken(parser)->type == TOKEN_SEMICOL) {
        node->decl.initializer = NULL;
        node->decl.type = "int";
        consume(parser, TOKEN_SEMICOL, "End of declaration");
        return node;
    }
    if (   currentToken(parser)->type != OP_ASSN
        && currentToken(parser)->type != TOKEN_SEMICOL) {
        reportError(parser, "Expected assignment or declaration");
        skip(parser);
        return NULL;
    }
    return NULL;
}

ASTNode *parseExpression(Parser* parser, int minPrecedence) {
    if (   currentToken(parser)->type != OP_COMPL
        && currentToken(parser)->type != OP_NEGATION
        && currentToken(parser)->type != OP_NEGATIONL
        && currentToken(parser)->type != LITERAL_INT
        && currentToken(parser)->type != TOKEN_OPAREN
        && currentToken(parser)->type != TOKEN_IDENTIFIER
        && currentToken(parser)->type != TOKEN_SEMICOL
        && currentToken(parser)->type != OP_INC
        && currentToken(parser)->type != OP_DEC) { 
        reportError(parser, "Invalid expression provided");
        skip(parser);
        return NULL;
    }

    ASTNode *left = parseFactor(parser);
    if (currentToken(parser)->type == TOKEN_SEMICOL) {
        return left;
    }

    while (currentToken(parser)) {
        Token *op = currentToken(parser);
        int opPrec = precedence(op->type);

        // Stop if operator's precedence is below the minimum,
        // or if we hit a closing parenthesis or semicolon.
        if (opPrec < minPrecedence || op->type == TOKEN_CPAREN || op->type == TOKEN_SEMICOL)
            break;

        if (op->type == OP_Q) {
            consume(parser, OP_Q, "Start of expression");
            ASTNode *trueC = parseExpression(parser, 0);
            if (!currentToken(parser) || currentToken(parser)->type != OP_COLON) {
                reportError(parser, "Expected ':' in conditional expression");
                skip(parser);
                return NULL;
            }
            consume(parser, OP_COLON, "Middle of expression");
            ASTNode *ternNode = newASTNode(AST_TERNARY);
            ASTNode *falseC = parseExpression(parser, opPrec);
            if (currentToken(parser)->type == OP_ASSN) {
                reportError(parser, "Cannot assign to expression");
                skip(parser);
                return NULL;
            }
            ternNode->ternary.condition = left;
            ternNode->ternary.trueCond = trueC;
            ternNode->ternary.falseCond = falseC;
            return ternNode;
        }   
        if (op->type == OP_ASSN) {
            ASTNode *binNode = parseBinary(parser);
            ASTNode *right = parseExpression(parser, opPrec);
            binNode->binary.left = left;
            binNode->binary.right = right;
            return binNode;
        }
        
        // Parse right-hand side expression with higher precedence.
        ASTNode *binNode = parseBinary(parser);
        if (binNode == NULL) break;
        ASTNode *right = parseExpression(parser, opPrec + 1);
        binNode->binary.left = left;
        binNode->binary.right = right;
        left = binNode;
    }

    return left;
}

ASTNode *parseFactor(Parser* parser) {
    ASTNode *node = newASTNode(AST_FACTOR);
    node->factor.expression = NULL;
    node->factor.unary = NULL;
    node->factor.constant = NULL;
    node->factor.identifier = NULL;
    node->factor.factor = NULL;
    if (currentToken(parser)->type == TOKEN_OPAREN) {
        consume(parser, TOKEN_OPAREN, "At expression start");
        node->factor.expression = parseExpression(parser, 0);
        consume(parser, TOKEN_CPAREN, "Expected closing bracket at end of expression");
        return node;
    }
    if (currentToken(parser)->type == OP_INC
        || currentToken(parser)->type == OP_DEC) {
        node->factor.unary = parseUnary(parser);
        node->factor.identifier = parseIdentifier(parser);
        return node;
    }
    if (currentToken(parser)->type == OP_NEGATION
        || currentToken(parser)->type == OP_NEGATIONL
        || currentToken(parser)->type == OP_COMPL) {
        node->factor.unary = parseUnary(parser);
        node->factor.factor = parseFactor(parser);
        return node;
    }   
    if (currentToken(parser)->type == LITERAL_INT) {
        node->factor.constant = parseConstant(parser);
        return node;
    }
    if (currentToken(parser)->type == TOKEN_IDENTIFIER) {
        node->factor.identifier = parseIdentifier(parser);
        if (currentToken(parser)->type == OP_INC 
            || currentToken(parser)->type == OP_DEC) {
            node->factor.unary = parseUnary(parser);
            node->factor.unary->unary.isPostfix = 1;
        }
        return node;
    }
    reportError(parser, "Invalid factor");
    return NULL;
}

ASTNode *parseUnary(Parser* parser) {
    if (currentToken(parser)->type != OP_COMPL 
        && currentToken(parser)->type != OP_NEGATION
        && currentToken(parser)->type != OP_NEGATIONL
        && currentToken(parser)->type != OP_INC
        && currentToken(parser)->type != OP_DEC) {
        reportError(parser, "Expected operator in expression");
        skip(parser);
        return NULL;
    }
    ASTNode *node = newASTNode(AST_UNARY);
    node->unary.value = strdup(currentToken(parser)->value);
    node->unary.isPostfix = 0;
    consume(parser, currentToken(parser)->type, "Before expression");
    return node;
}

ASTNode *parseBinary(Parser* parser) {
    if (currentToken(parser)->type < OP_NEGATION || currentToken(parser)->type >= TOKEN_EOF) {
        reportError(parser, "Expected operator in expression");
        skip(parser);
        return NULL;
    }
    if (currentToken(parser)->type == OP_COLON || currentToken(parser)->type == OP_Q) {
        return NULL;
    }
    ASTNode *node = newASTNode(AST_BINARY);
    node->binary.value = strdup(currentToken(parser)->value);
    consume(parser, currentToken(parser)->type, "Before expression");
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

ASTNode *parseIdentifier(Parser *parser) {
    if (currentToken(parser)->type != TOKEN_IDENTIFIER) {
        reportError(parser, "Expected identifier");
        skip(parser);
        return NULL;
    }
    ASTNode *node = newASTNode(AST_IDENTIFIER);
    node->identifier.value = strdup(currentToken(parser)->value);
    consume(parser, TOKEN_IDENTIFIER, "After identifier");
    if (currentToken(parser)->type == OP_INC) {

    }
    return node;

}

/* --- AST Printing for Debug --- */
void printAST(ASTNode *node, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    printf("|__");
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
            if (node->statement.expression != NULL ) printAST(node->statement.expression, indent + 1);
            if (node->statement.declaration != NULL ) printAST(node->statement.declaration, indent + 1);
            if (node->statement.compound != NULL ) printAST(node->statement.compound, indent + 1);
            if (node->statement.retn != NULL ) printAST(node->statement.retn, indent + 1);
            if (node->statement.ifstatement != NULL ) printAST(node->statement.ifstatement, indent + 1);
            break;
        case AST_EXPRESSION:
            printf("Terms:\n");
            if (node->expression.term != NULL) {
                printAST(node->expression.term, indent);
            } 
            if (node->expression.binary != NULL) {
                printAST(node->expression.binary, indent + 1);
            }
            break;
        case AST_DECL:
            printf("Declaration:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("|__");
            if (node->decl.type != NULL ) {
                printf("Type: %s\n", node->decl.type);
            }
            if (node->decl.identifier != NULL) {
                printAST(node->decl.identifier, indent + 1);
            }
            if (node->decl.initializer != NULL) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("|__");
                printf("Initialized as:\n");
                printAST(node->decl.initializer, indent + 2);
            }
            break;
        case AST_RETURN:
            printf("Returns:\n");
            if (node->retn.expression != NULL) {
                printAST(node->retn.expression, indent + 1);
            }
            break;
        case AST_IF:
            printf("Condition:\n");
            printAST(node->ifstmt.condition, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("|__");
            printf("Body:\n");
            printAST(node->ifstmt.body, indent + 1);
            if (node->ifstmt.elsestmt != NULL) {
                for (int i = 0; i < indent; i++) printf("  ");
                printf("|__");
                printf("Else:\n");
                printAST(node->ifstmt.elsestmt, indent + 1);
            }
            break;
        case AST_TERNARY:
            printf("Condition:\n");
            printAST(node->ternary.condition, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("|__");
            printf("True condition:\n");
            printAST(node->ternary.trueCond, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("|__");
            printf("False condition:\n");
            printAST(node->ternary.falseCond, indent + 1);
            break;
        case AST_FACTOR:
            printf("Factor:\n");
            if (node->factor.expression != NULL) printAST(node->factor.expression, indent + 1); 
            if (node->factor.unary != NULL) printAST(node->factor.unary, indent + 1);   
            if (node->factor.constant != NULL) printAST(node->factor.constant, indent + 1); 
            if (node->factor.factor != NULL) printAST(node->factor.factor, indent + 1);
            if (node->factor.identifier != NULL) printAST(node->factor.identifier, indent + 1);
            break;
        case AST_CONSTANT:
            printf("Constant: %s\n", node->constant.value);
            break;
        case AST_UNARY:
            printf("Operator: %s\n", node->unary.value);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("|__");
            printf("Postfix?: %s\n", node->unary.isPostfix ? "true" : "false");
            break;
        case AST_BINARY:
            printf("Left:\n");
            printAST(node->binary.left, indent + 1);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("|__");
            printf("Operator:%s\n", node->binary.value);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("|__");
            printf("Right:\n");
            printAST(node->binary.right, indent + 1);
            break;
        case AST_IDENTIFIER:
            printf("Name: %s\n", node->identifier.value);
            break;
        default:
            printf("Unknown AST Node\n");
            printf("Value: %d\n", node->type);
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
        case AST_FACTOR:
            if ( node->factor.expression != NULL ) {
                freeAST(node->factor.expression);
                node->factor.expression = NULL;
            }
            if ( node->factor.unary != NULL ) {
                freeAST(node->factor.unary);
                node->factor.unary = NULL;
            }
            if ( node->factor.constant != NULL ) {
                freeAST(node->factor.constant);
                node->factor.constant = NULL;
            }
            if ( node->factor.factor != NULL ) {
                freeAST(node->factor.factor);
                node->factor.factor = NULL;
            }
            break;
        case AST_UNARY:
            if (node->unary.value != NULL) {
                free(node->unary.value);
                node->unary.value = NULL;
            }
            break;
        case AST_CONSTANT:
            if (node->unary.value != NULL) {
                free(node->constant.value);
                node->constant.value = NULL;
            }
            break;
        case AST_BINARY:
            if (node->binary.left != NULL) {
                freeAST(node->binary.left);
                node->binary.left = NULL;
            }
            if (node->binary.right != NULL) {
                freeAST(node->binary.right);
                node->binary.right = NULL;
            }
            if (node->binary.value != NULL) {
                free(node->binary.value);
                node->binary.value = NULL;
            }
            break;
        default:
            break;
    }
    free(node);
}

// Free the array of tokens.
void freeTokens(Token **tokens, int tokenCount) {
    for (int i = 0; i < tokenCount; i++) {
        if (tokens[i]->type == KEYW_INT
        ||  tokens[i]->type == KEYW_RETURN
        ||  tokens[i]->type == LITERAL_INT
        ||  tokens[i]->type == TOKEN_IDENTIFIER) {
            free(tokens[i]->value);
            tokens[i]->value = NULL;
        }
        if (tokens[i] != NULL) {
            free(tokens[i]);
            tokens[i] = NULL;
        }
    }
    free(tokens);
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

int lineCount = 1;
const char* keywords[] = {
	"return",
	"int",
	"if",
	"else",
	NULL
};

int isKeyword(const char* word) {
    for (int i = 0; keywords[i] != NULL; i++) {
		if (strcmp(word, keywords[i]) == 0) {
			return 1; // It's a keyword
		}
	}
    return 0; // Not a keyword
}

Lexer* createLexer(const char* input){
	Lexer* l = malloc(sizeof(Lexer));
	l->input = input;
	l->pos = 0;
	return l;	
}

Token* createToken(TokenType type, char* value){
	Token* t = malloc(sizeof(Token));
	t->type = type;
	t->value = value;
	t->line = lineCount;
	return t;
}

void freeToken(Token* token){
	free(token->value);
	free(token);
}

// Main lexing logic:
Token* lexerNextToken(Lexer* l) {
	while(l->pos < strlen(l->input)){
		char c = l->input[l->pos];

		// Newline:
		if (c == '\n') {
			lineCount += 1;
			l->pos++;
			continue;
		}

		// Spaces, newlines, tabs:
		if (isspace(c)) {
			l->pos++;
			continue;	
		}

		// Integer literal:
		if (isdigit(c)) {
			size_t s = l->pos;
			while(l->pos < strlen(l->input) && isdigit(l->input[l->pos])) {
				l->pos++;
			}
			char* n = strndup(l->input + s, l->pos - s);
			return createToken(LITERAL_INT, n);
		}

		// Parantheses:
		if (c == '(') {
			l->pos++;
			return createToken(TOKEN_OPAREN, "(");
		}
		if (c == ')') {
			l->pos++;
			return createToken(TOKEN_CPAREN, ")");
		}

		// Braces:
		if (c == '{') {
			l->pos++;
			return createToken(TOKEN_OBRACE, "{");
		}
		if (c == '}') {
			l->pos++;
			return createToken(TOKEN_CBRACE, "}");
		}

		// Semicolon:
		if (c == ';') {
			l->pos++;
			return createToken(TOKEN_SEMICOL, ";");
		}

		// Negation, Bitwise comp.:
		if (c == '-') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '=') {
				l->pos++;
				return createToken(OP_DECEQ, "-=");
			}
			if (l->pos < strlen(l->input) && l->input[l->pos] == '-') {
				l->pos++;
				return createToken(OP_DEC, "--");
			}
			return createToken(OP_NEGATION, "-");
		}
		if (c == '~') {
			l->pos++;
			return createToken(OP_COMPL, "~");
		}
		if (c == '!') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '=') {
				l->pos++;
				return createToken(OP_NOTEQ, "!=");
			}
			return createToken(OP_NEGATIONL, "!");
		}

		// Operators:
		if (c == '+') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '=') {
				l->pos++;
				return createToken(OP_INCEQ, "+=");
			}
			if (l->pos < strlen(l->input) && l->input[l->pos] == '+') {
				l->pos++;
				return createToken(OP_INC, "++");
			}
			return createToken(OP_ADD, "+");
		}
		if (c == '*') {
			l->pos++;
			return createToken(OP_MUL, "*");
		}
		if (c == '/') {
			l->pos++;
			return createToken(OP_DIV, "/");
		}

		// Logical operators:
		if (c == '=') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '=') {
				l->pos++;
				return createToken(OP_EQ, "==");
			}
			return createToken(OP_ASSN, "=");
		}
		if (c == '&') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '&') {
				l->pos++;
				return createToken(OP_AND, "&&");
			}
		}
		if (c == '|') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '|') {
				l->pos++;
				return createToken(OP_OR, "||");
			}
		}
		if (c == '<') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '=') {
				l->pos++;
				return createToken(OP_LESSEQ, "<=");
			}
			return createToken(OP_LESS, "<");
		}
		if (c == '>') {
			l->pos++;
			if (l->pos < strlen(l->input) && l->input[l->pos] == '=') {
				l->pos++;
				return createToken(OP_GREATEREQ, ">=");
			}
			return createToken(OP_GREATER, ">");
		}

		// Conditional operator:
		if (c == '?') {
			l->pos++;
			return createToken(OP_Q, "?");
		}
		if (c == ':') {
			l->pos++;
			return createToken(OP_COLON, ":");
		}

		// Identifier/Keyword:
		if (isalnum(c)) {
			size_t s = l->pos;
			while ( (l->pos < strlen(l->input)) 
					&& isalnum(l->input[l->pos]) ) {
				l->pos++;
			}
			char* str = strndup(l->input + s, l->pos - s);
			if (isKeyword(str)){
				if (strcmp(str, "int") == 0) {
					return createToken(KEYW_INT, str);
				}
				if (strcmp(str, "return") == 0) {
					return createToken(KEYW_RETURN, str);
				}
				if (strcmp(str, "if") == 0) {
					return createToken(KEYW_IF, str);
				}
				if (strcmp(str, "else") == 0) {
					return createToken(KEYW_ELSE, str);
				}
			}
			return createToken(TOKEN_IDENTIFIER, str);
		}

		l->pos++;
		return createToken(TOKEN_INVALID, "Invalid");
	}
	return NULL;
}

Token** lex(const char* filename, int* token_count) {
    // Open the file
    FILE* file = fopen(filename, "r");
    if (!file) {
		perror("Could not open file");
		return NULL;
	}

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* input = malloc(length + 1);
    fread(input, 1, length, file);
    input[length] = '\0'; // Null-terminate the string
    fclose(file);

    // Create the lexer
    Lexer* lexer = createLexer(input);
    Token** tokens = NULL; // Start with a NULL pointer
    *token_count = 0;

    // Tokenize the input
    Token* token;
	do {
		token = lexerNextToken(lexer);
		if (!token) {
			break;
		}
		// Append the token to the array
		tokens = realloc(tokens, sizeof(Token*) * (*token_count + 1)); // Resize to hold one more token
		tokens[*token_count] = token; // Add the new token
		//printf("Current token: Type=%d, Value='%s'\n", tokens[*token_count]->type, tokens[*token_count]->value);
		//printf("Current token index: %d\n", *token_count);
		(*token_count)++;
	} while (token != NULL); 

    //free(token); // Free the last EOF token
    free(lexer);
    free(input); // Free the input string

    return tokens; // Return the array of tokens
}

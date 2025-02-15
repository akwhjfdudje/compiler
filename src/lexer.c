#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

const char* keywords[] = {
	"int",
	"return",
	NULL
};

int is_keyword(const char* word) {
    for (int i = 0; keywords[i] != NULL; i++) {
		if (strcmp(word, keywords[i]) == 0) {
			return 1; // It's a keyword
		}
	}
    return 0; // Not a keyword
}

Lexer* create_lexer(const char* input){
	Lexer* l = malloc(sizeof(Lexer));
	l->input = input;
	l->pos = 0;
	return l;	
}

Token* create_token(TokenType type, const char* value){
	Token* t = malloc(sizeof(Token));
	t->type = type;
	t->value = strdup(value);
	return t;
}

void free_token(Token* token){
	free(token->value);
	free(token);
}

// Main lexing logic:
Token* lexer_next_token(Lexer* l) {
	while(l->pos < strlen(l->input)){
		char c = l->input[l->pos];

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
			const char* n = strndup(l->input + s, l->pos - s);
			return create_token(LITERAL_INT, n);
		}

		// Parantheses:
		if (c == '(') {
			l->pos++;
			return create_token(TOKEN_OPAREN, "(");
		}
		if (c == ')') {
			l->pos++;
			return create_token(TOKEN_CPAREN, ")");
		}

		// Braces:
		if (c == '{') {
			l->pos++;
			return create_token(TOKEN_OBRACE, "{");
		}
		if (c == '}') {
			l->pos++;
			return create_token(TOKEN_CBRACE, "}");
		}

		// Semicolon:
		if (c == ';') {
			l->pos++;
			return create_token(TOKEN_SEMICOL, ";");
		}

		// Identifier/Keyword:
		if (isalpha(c)) {
			size_t s = l->pos;
			while ( (l->pos < strlen(l->input)) && isalpha(l->input[l->pos])) {
				l->pos++;
			}
			char* str = strndup(l->input + s, l->pos - s);
			if (is_keyword(str)){
				if (strcmp(str, "int")) {
					return create_token(KEYW_INT, str);
				}
				if (strcmp(str, "return")) {
					return create_token(KEYW_RETURN, str);
				}
			}
			return create_token(TOKEN_IDENTIFIER, str);
		}
		l->pos++;
		return create_token(TOKEN_INVALID, "Invalid");
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
    Lexer* lexer = create_lexer(input);
    Token** tokens = NULL; // Start with a NULL pointer
    *token_count = 0;

    // Tokenize the input
    Token* token;
    do {
		token = lexer_next_token(lexer);
		// Append the token to the array
		tokens = realloc(tokens, sizeof(Token*) * (*token_count + 1)); // Resize to hold one more token
		tokens[*token_count] = token; // Add the new token
		(*token_count)++;
	} while (token != NULL);

    free(token); // Free the last EOF token
    free(lexer);
    free(input); // Free the input string

    return tokens; // Return the array of tokens
}

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

    // Print the tokens
    for (int i = 0; i < token_count && tokens[i] != NULL; i++) {
		printf("Token: Type=%d, Value='%s'\n", tokens[i]->type, tokens[i]->value);
		free_token(tokens[i]); // Free each token
	}
    free(tokens); // Free the array of tokens

	return 0;
}

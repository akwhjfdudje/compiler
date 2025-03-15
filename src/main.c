#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "parser.h"
#include "generator.h"
#include "lexer.h"

const char *getBasename(const char *filename);
char *getDirectory(const char *filepath);

int main(int argc, char** argv) {

	// TODO: add DEBUG ifdef for, you know, debugging.
	if (argc < 2) {
  		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	const char *cFile = argv[1];
	char *dir = getDirectory(cFile);
	const char *basename = getBasename(cFile);

	/* Construct full output path */
	char outputPath[512];
	snprintf(outputPath, sizeof(outputPath), "%s/%s", dir, basename);

    int token_count = 0;
    Token** tokens = lex(argv[1], &token_count);
    if (!tokens) {
		return 1; // Error occurred in lex()
	}

	//printf("%d\n", token_count);
	
	#ifdef DEBUG
	for (int i = 0; i < token_count && tokens[i] != NULL; i++) {
		printf("Token: Type=%d, Value='%s'\n", tokens[i]->type, tokens[i]->value);
	}
	#endif
	
    // Initialize the parser context.
	Parser parser;
	parser.tokens = tokens;
	parser.currentIndex = 0;
	parser.tokenCount = token_count;
	parser.errorFlag = 0;

	// Parse the tokens into an AST.
	ASTNode *ast = parseProgram(&parser);
	if (parser.errorFlag) {
		printf("Do better.\n");
		return 1;
	}

	// Print the tree.
	#ifdef DEBUG
	//printf("Parsing complete!\n\nAST:\n");
	printAST(ast, 0);
	#endif

	// Generate x86 code.

	CodeGenerator generator;
	initStringBuffer(&generator.sb);
	generateX86(&generator, ast);
  
	// Put into a file.
	FILE *file = fopen("asm.s", "w");
	if (!file) {
		perror("Failed to open asm.s for writing");
		return 1;
	}
	fprintf(file, "%s", generator.sb.data);
	fclose(file);	
	#ifdef DEBUG
	printf("Assembly code written to asm.s:\n%s\n", generator.sb.data);
	#endif

	const char *exeName = outputPath;
	char command[1024];
	snprintf(command, sizeof(command), "gcc -m32 asm.s -o %s", outputPath);
    int ret = system(command);
	if (ret != 0) {
		fprintf(stderr, "Compilation failed with exit code %d\n", ret);
		return ret;
	}

	//printf("Compilation succeeded: executable '%s' created.\n", exeName);
	return 0;
	
	char exec[256];
	snprintf(exec, sizeof(exec), "./%s", exeName);
    ret = system(exec);
	if (ret != 0) {
		fprintf(stderr, "Execution failed with %d\n", ret);
		return ret;
	}
	char rm[256];
	snprintf(rm, sizeof(rm), "rm %s", exeName);
    ret = system(rm);
	if (ret != 0) {
		fprintf(stderr, "Deletion failed with %d\n", ret);
		return ret;
	}

	free(generator.sb.data);
	freeAST(ast);
	freeTokens(tokens, token_count);
	free((void *)basename);
	free((void *)dir);
	return 0;
}

const char *getBasename(const char *filename) {
	const char *base = strrchr(filename, '/'); // Get last part after '/'
	base = (base) ? base + 1 : filename;       // If '/' found, move past it
	char *dot = strrchr(base, '.');            // Find last '.'
	if (dot && dot != base) {                  // Ensure dot isn't the first char
		size_t len = dot - base;
		char *name = malloc(len + 1);
		if (!name) {
			perror("malloc failed");
			exit(1);
		}
		strncpy(name, base, len);
		name[len] = '\0';
		return name;
	}
	return strdup(base); // If no '.', return full name
}

char *getDirectory(const char *filepath) {
    char *dir = strdup(filepath); // Duplicate string to modify
    if (!dir) {
		perror("strdup failed");
		exit(1);
	}
    char *lastSlash = strrchr(dir, '/');
    if (lastSlash) {
		*lastSlash = '\0'; // Cut off at last '/'
		return dir;
	}
	free(dir);
	return strdup("."); // Default to current directory if no '/'
}

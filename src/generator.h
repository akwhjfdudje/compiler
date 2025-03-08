#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "lexer.h"

typedef struct {
	char *data;
	size_t length;
	size_t capacity;
} StringBuffer;

typedef struct {
	StringBuffer sb; // Accumulates generated code.
} CodeGenerator;

void initStringBuffer(StringBuffer *sb);
void appendString(StringBuffer *sb, const char *str);
void appendFormat(StringBuffer *sb, const char *format, ...);
void generateX86(CodeGenerator *gen, ASTNode *node);

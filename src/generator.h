#include "parser.h"
#ifndef GENERATOR_H 
#define GENERATOR_H

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
int generateX86(CodeGenerator *gen, ASTNode *node);
#endif

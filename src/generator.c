#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "generator.h"

void initStringBuffer(StringBuffer *sb) {
	sb->capacity = 256;
	sb->length = 0;
	sb->data = malloc(sb->capacity);
	sb->data[0] = '\0';
}

void appendString(StringBuffer *sb, const char *str) {
	size_t addLen = strlen(str);
	if (sb->length + addLen + 1 > sb->capacity) {
		while (sb->length + addLen + 1 > sb->capacity) {
			sb->capacity *= 2;
		}
		sb->data = realloc(sb->data, sb->capacity);
	}
	strcat(sb->data, str);
	sb->length += addLen;
}

void appendFormat(StringBuffer *sb, const char *format, ...) {
	char temp[512];
	va_list args;
	va_start(args, format);
	vsnprintf(temp, sizeof(temp), format, args);
	va_end(args);
	appendString(sb, temp);
}

void generateX86(CodeGenerator *gen, ASTNode *node) {
	if (!node)
		return;

	switch (node->type) {
		case AST_PROGRAM: {
			for (int i = 0; i < node->program.functionCount; i++) {
				generateX86(gen, node->program.functions[i]);
				appendString(&gen->sb, "\n");
			}
			break;
		}
		case AST_FUNCTION: {
			// Emit global directive and function label.
			appendFormat(&gen->sb, "    .globl %s\n", node->function.name);
			appendFormat(&gen->sb, "%s:\n", node->function.name);
			generateX86(gen, node->function.body);
			break;
		}
		case AST_BLOCK: {
			// Process each statement in the block.
			for (int i = 0; i < node->block.statementCount; i++) {
				generateX86(gen, node->block.statements[i]);
			}
			break;
		}
		case AST_STATEMENT: {
			appendString(&gen->sb, "    # statement\n");
			generateX86(gen, node->statement.expression);  // Generate code for the return expression.
			break;
		}
		case AST_EXPRESSION: {
			generateX86(gen, node->expression.constant);  // Generate code for the constant
			break;
		}
		case AST_CONSTANT: {
			appendFormat(&gen->sb, "    movl $%s, %%eax\n", node->constant.value);
			appendString(&gen->sb, "    ret\n");
			break;
		}
		default:
			appendString(&gen->sb, "    # Unknown AST Node\n");
			break;
	}
}

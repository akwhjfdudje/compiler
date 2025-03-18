#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "parser.h"
#include "generator.h"
#include "data.h"

int labelCount = 0;
HashMap* varmap; 
int stackIndex = 0;
int m;
int cFail;
int ret;
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

void makeLabel(char *label, int id) {
	snprintf(label, 256, "_LLLL%d", id);
}

int generateX86(CodeGenerator *gen, ASTNode *node) {
	if (!node)
		return 1;

	switch (node->type) {
		case AST_PROGRAM: {
			for (int i = 0; i < node->program.functionCount; i++) {
				generateX86(gen, node->program.functions[i]);
			}
			break;
		}
		case AST_FUNCTION: {
			// Set start of stack pointer:
			stackIndex = 0;
			// Emit global directive and function label.
			appendFormat(&gen->sb, "    .globl %s\n", node->function.name);
			appendFormat(&gen->sb, "%s:\n", node->function.name);
			appendFormat(&gen->sb, "    push     %%ebp\n");
			appendFormat(&gen->sb, "    movl     %%esp, %%ebp\n");
			generateX86(gen, node->function.body);
			appendFormat(&gen->sb, "    movl     %%ebp, %%esp\n");
			appendFormat(&gen->sb, "    pop      %%ebp\n");
			if (!(strcmp(node->function.name, "main")) && !(ret)) {
				// Section 5.1.2.2.3, C11:
				appendString(&gen->sb, "    movl     $0, %eax\n");
			}
			appendString(&gen->sb, "    ret\n");
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
			if (node->statement.expression != NULL) {
				generateX86(gen, node->statement.expression);  // Generate code for an expression.
			}
			if (node->statement.declaration != NULL) {
				generateX86(gen, node->statement.declaration);  // Generate code for a declaration.
			}
			if (node->statement.retn != NULL) {
				generateX86(gen, node->statement.retn); // Generate code for the return expression. 
			}
			break;
		}
		case AST_DECL: {
			// Check existence:
			if (!m) {
				varmap = createHashmap(64);
				m = 1;
			}
			const char *id = node->decl.identifier->identifier.value;
			if (getHash(varmap, id) != -1) {
				// TODO: make better compiler errors...
				printf("Compile error: identifier already declared at %s\n", id);
				cFail = 1;
			}
			if (node->decl.initializer != NULL) {
				generateX86(gen, node->decl.initializer);
			}
			appendString(&gen->sb, "    pushl    %eax\n");
			stackIndex -= 4;	
			insertHash(varmap, id, stackIndex);
			break;
		}
		case AST_RETURN: {
			// Set a return flag:
			ret = 1;
			generateX86(gen, node->retn.expression);
			appendString(&gen->sb, "    movl      %eax, %eax\n");
		}
		case AST_FACTOR: {
			if (node->factor.factor != NULL) generateX86(gen, node->factor.factor); // Generate code for the subfactor
			if (node->factor.expression != NULL ) generateX86(gen, node->factor.expression);  // Generate code for a subexpression
			if (node->factor.unary != NULL ) generateX86(gen, node->factor.unary);  // Generate code for a unary operator 
			if (node->factor.constant != NULL ) generateX86(gen, node->factor.constant);  // Generate code for the constant
			if (node->factor.identifier != NULL ) generateX86(gen, node->factor.identifier);
			break;
		}
		case AST_BINARY: {
			/*
				x86 code for a binary addition operation:
				<CODE FOR e1 GOES HERE>
				push %eax ; save value of e1 on the stack
				<CODE FOR e2 GOES HERE>
				pop %ecx ; pop e1 from the stack into ecx
				addl %ecx, %eax ; add e1 to e2, save results in eax

				x86 code for comparisons:
				<CODE FOR e1 GOES HERE>
				push   %eax          ; save value of e1 on the stack
				<CODE FOR e2 GOES HERE>
				pop    %ecx          ; pop e1 from the stack into ecx - e2 is already in eax
				cmpl   %eax, %ecx    ;set ZF on if e1 == e2, set it off otherwise
				movl   $0, %eax      ;zero out EAX (doesn't change FLAGS)
				sete   %al           ;set AL register (the lower byte of EAX) to 1 iff ZF is on
			*/
			if (!(strcmp(node->binary.value, "+"))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    addl      %ecx, %eax\n");
			}
			if (!(strcmp(node->binary.value, "-"))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    subl      %eax, %ecx\n");
				appendString(&gen->sb, "    movl      %ecx, %eax\n");
			}
			if (!(strcmp(node->binary.value, "*"))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    imul      %ecx, %eax\n");
			}
			if (!(strcmp(node->binary.value, "/"))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    xorl      %ecx, %eax\n");
				appendString(&gen->sb, "    xorl      %eax, %ecx\n");
				appendString(&gen->sb, "    xorl      %ecx, %eax\n");
				appendString(&gen->sb, "    cdq\n");
				appendString(&gen->sb, "    idivl     %ecx\n");
			}
			if (!(strcmp(node->binary.value, "<="))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    cmpl      %eax, %ecx\n");
				appendString(&gen->sb, "    movl      $0, %eax\n");
				appendString(&gen->sb, "    setle     %al\n");
			}
			if (!(strcmp(node->binary.value, "<"))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    cmpl      %eax, %ecx\n");
				appendString(&gen->sb, "    movl      $0, %eax\n");
				appendString(&gen->sb, "    setl      %al\n");
			}
			if (!(strcmp(node->binary.value, ">="))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    cmpl      %eax, %ecx\n");
				appendString(&gen->sb, "    movl      $0, %eax\n");
				appendString(&gen->sb, "    setge     %al\n");
			}
			if (!(strcmp(node->binary.value, ">"))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    cmpl      %eax, %ecx\n");
				appendString(&gen->sb, "    movl      $0, %eax\n");
				appendString(&gen->sb, "    setg      %al\n");
			}
			if (!(strcmp(node->binary.value, "=="))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    cmpl      %eax, %ecx\n");
				appendString(&gen->sb, "    movl      $0, %eax\n");
				appendString(&gen->sb, "    sete      %al\n");
			}
			if (!(strcmp(node->binary.value, "!="))) {
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    push      %eax\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    pop       %ecx\n");
				appendString(&gen->sb, "    cmpl      %eax, %ecx\n");
				appendString(&gen->sb, "    movl      $0, %eax\n");
				appendString(&gen->sb, "    setne     %al\n");
			}
			if (!(strcmp(node->binary.value, "&&"))) {
				labelCount++;
				char label[256];
				makeLabel(label, labelCount);
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    cmpl     $0, %eax\n");
				appendString(&gen->sb, "    jne     ");
				appendString(&gen->sb, label);
				appendString(&gen->sb, "\n");
				labelCount++;
				char label2[256];
				makeLabel(label2, labelCount);
				appendString(&gen->sb, "    jmp   ");
				appendString(&gen->sb, label2);
				appendString(&gen->sb, "\n");
				appendString(&gen->sb, label);
				appendString(&gen->sb, ":\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    cmpl     $0, %eax\n");
				appendString(&gen->sb, "    movl     $0, %eax\n");
				appendString(&gen->sb, "    setne    %al\n");
				appendString(&gen->sb, label2);
				appendString(&gen->sb, ":\n");
			}
			if (!(strcmp(node->binary.value, "||"))) {
				labelCount++;
				char label[256];
				makeLabel(label, labelCount);
				generateX86(gen, node->binary.left);
				appendString(&gen->sb, "    cmpl     $0, %eax\n");
				appendString(&gen->sb, "    je     ");
				appendString(&gen->sb, label);
				appendString(&gen->sb, "\n");
				appendString(&gen->sb, "    movl     $1, %eax\n");
				labelCount++;
				char label2[256];
				makeLabel(label2, labelCount);
				appendString(&gen->sb, "    jmp    ");
				appendString(&gen->sb, label2);
				appendString(&gen->sb, "\n");
				appendString(&gen->sb, label);
				appendString(&gen->sb, ":\n");
				generateX86(gen, node->binary.right);
				appendString(&gen->sb, "    cmpl     $0, %eax\n");
				appendString(&gen->sb, "    movl     $0, %eax\n");
				appendString(&gen->sb, "    setne    %al\n");
				appendString(&gen->sb, label2);
				appendString(&gen->sb, ":\n");
			}
			if (!(strcmp(node->binary.value, "="))) {
				generateX86(gen, node->binary.right);
				struct ASTNode* l = node->binary.left;
				if (l->type != AST_FACTOR) {
					printf("Compile error: expected variable, found expression.\n");
					cFail = 1;
					return 1;
				}
				if (l->factor.identifier == NULL) {
					printf("Compile error: invalid assignment; expected variable.\n");
					cFail = 1;
					return 1;
				}
				const char *id = l->factor.identifier->identifier.value;
				int varOffset = getHash(varmap, id);
				char offset[32];
				snprintf(offset, 32, "    movl     %%eax, %d(%%ebp)\n", varOffset);
				appendString(&gen->sb, offset);
			}
			break;
		}
		case AST_CONSTANT: {
			appendFormat(&gen->sb, "    movl     $%s, %%eax\n", node->constant.value);
			break;
		}
		case AST_UNARY: {
			if (strcmp(node->unary.value, "-") == 0) {
				appendString(&gen->sb, "    neg %eax\n");
			}
			if (strcmp(node->unary.value, "~") == 0) {
				appendString(&gen->sb, "    not %eax\n");
			}
			if (strcmp(node->unary.value, "!") == 0) {
				appendString(&gen->sb, "    cmpl $0, %eax\n");
				appendString(&gen->sb, "    movl $0, %eax\n");
				appendString(&gen->sb, "    sete %al\n");
			}
			break;
		}
		case AST_IDENTIFIER: {
			int varOffset = getHash(varmap, node->identifier.value);
			if (varOffset == -1) {
				printf("Compile error: identifier does not exist.\n");
				cFail = 1;
			}
			char offset[32];
			snprintf(offset, 32, "    movl      %d(%%ebp), %%eax\n", varOffset);
			appendString(&gen->sb, offset);
			break;
		}
		default:
			appendString(&gen->sb, "    # Unknown AST Node\n");
			break;
	}
	return cFail;
}

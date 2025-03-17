#ifndef DATA_H
#define DATA_H
#include <stdlib.h>
#include <string.h>

// Define the structure for each entry in the HashMap
typedef struct {
    char* key;
    void* value;
} HashMapEntry;

// Define the structure for the HashMap
typedef struct {
    size_t size;
    size_t capacity;
    HashMapEntry* entries;
} HashMap;

// Stack structure definition
typedef struct {
    int *array;        
    size_t size;      
    size_t capacity;   
} Stack;

// Hashmap: 
HashMap* createHashmap(size_t capacity);
void freeHashmap(HashMap* hashmap);
void* getHash(HashMap* hashmap, const char* key);
int insertHash(HashMap* hashmap, const char* key, void* value);
int removeHash(HashMap* hashmap, const char* key);

// Stack:
Stack* createStack();
void freeStack(Stack* stack);
int stackPush(Stack* stack, int value);
int stackPop(Stack* stack, int* value);
int stackPeek(Stack* stack, int* value);
int stackIsEmpty(Stack* stack);
size_t stackSize(Stack* stack);
void stackResize(Stack* stack, size_t newCapacity);

#endif

#ifndef DATA_H
#define DATA_H
#include <stdlib.h>
#include <string.h>

// Define the structure for each entry in the HashMap
typedef struct {
    char* key;
    int value;
} HashMapEntry;

// Define the structure for the HashMap
typedef struct {
    size_t size;
    size_t capacity;
    HashMapEntry* entries;
} HashMap;

// Stack structure definition
typedef struct {
    HashMap** array;        
    size_t size;      
    size_t capacity;   
} Stack;

// Hashmap: 
HashMap* createHashmap(size_t capacity);
void freeHashmap(HashMap* hashmap);
int getHash(HashMap* hashmap, const char* key);
int insertHash(HashMap* hashmap, const char* key, int value);
int removeHash(HashMap* hashmap, const char* key);
int copyHashMap(HashMap* dest, const HashMap* src);

// Stack:
Stack* createStack();
void freeStack(Stack* stack);
int stackPush(Stack* stack, HashMap* value);
int stackPop(Stack* stack, HashMap** value);
int stackPeek(Stack* stack, HashMap** value);
int stackIsEmpty(Stack* stack);
size_t stackSize(Stack* stack);
void stackResize(Stack* stack, size_t newCapacity);

#endif

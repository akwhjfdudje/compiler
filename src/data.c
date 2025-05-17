#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Hash function to compute index for a key
static size_t hash(const char* key, size_t capacity) {
    size_t hash_value = 0;
    for (size_t i = 0; key[i] != '\0'; i++) {
        hash_value = (hash_value * 31) + key[i];
    }
    return hash_value % capacity;
}

// Create a new HashMap with the specified capacity
HashMap* createHashmap(size_t capacity) {
    HashMap* hashmap = (HashMap*)malloc(sizeof(HashMap));
    hashmap->capacity = capacity;
    hashmap->size = 0;
    hashmap->entries = (HashMapEntry*)calloc(capacity, sizeof(HashMapEntry));
    return hashmap;
}

// Free the memory allocated for the HashMap
void freeHashmap(HashMap* hashmap) {
    if (hashmap) {
        for (size_t i = 0; i < hashmap->capacity; i++) {
            if (hashmap->entries[i].key != NULL) {
                free(hashmap->entries[i].key);
            }
        }
        free(hashmap->entries);
        free(hashmap);
    }
}

// Get the value associated with a key in the HashMap
int getHash(HashMap* hashmap, const char* key) {
    size_t index = hash(key, hashmap->capacity);
    for (size_t i = 0; i < hashmap->capacity; i++) {
        size_t probe_index = (index + i) % hashmap->capacity;
        if (hashmap->entries[probe_index].key == NULL) {
            return -1; // Key not found
        }
        if (strcmp(hashmap->entries[probe_index].key, key) == 0) {
            return hashmap->entries[probe_index].value;
        }
    }
    return -1; // Key not found
}

// Insert or update a key-value pair in the HashMap
int insertHash(HashMap* hashmap, const char* key, int value) {
    if (hashmap->size >= hashmap->capacity) {
        return -1; // HashMap is full
    }

    size_t index = hash(key, hashmap->capacity);
    for (size_t i = 0; i < hashmap->capacity; i++) {
        size_t probe_index = (index + i) % hashmap->capacity;
        if (hashmap->entries[probe_index].key == NULL) {
            hashmap->entries[probe_index].key = strdup(key);
            hashmap->entries[probe_index].value = value;
            hashmap->size++;
            return 0;
        }
        if (strcmp(hashmap->entries[probe_index].key, key) == 0) {
            hashmap->entries[probe_index].value = value;
            return 0;
        }
    }
    return -1; // HashMap is full (shouldn't reach here if size check is correct)
}

// Remove a key-value pair from the HashMap
int removeHash(HashMap* hashmap, const char* key) {
    size_t index = hash(key, hashmap->capacity);
    for (size_t i = 0; i < hashmap->capacity; i++) {
        size_t probe_index = (index + i) % hashmap->capacity;
        if (hashmap->entries[probe_index].key == NULL) {
            return -1; // Key not found
        }
        if (strcmp(hashmap->entries[probe_index].key, key) == 0) {
            free(hashmap->entries[probe_index].key);
            hashmap->entries[probe_index].key = NULL;
            hashmap->entries[probe_index].value = -1;
            hashmap->size--;
            return 0;
        }
    }
    return -1; // Key not found
}

// Copy contents from src hashmap to dest hashmap
int copyHashMap(HashMap* dest, const HashMap* src) {
    if (dest == NULL || src == NULL) return -1;

    // Check that the destination has enough capacity
    if (dest->capacity < src->capacity) {
        return -1; // Not enough capacity in the destination
    }

    // Clear destination first
    for (size_t i = 0; i < dest->capacity; i++) {
        if (dest->entries[i].key != NULL) {
            free(dest->entries[i].key);
            dest->entries[i].key = NULL;
            dest->entries[i].value = -1;
        }
    }
    dest->size = 0;

    // Copy over entries from source
    for (size_t i = 0; i < src->capacity; i++) {
        if (src->entries[i].key != NULL) {
            insertHash(dest, src->entries[i].key, src->entries[i].value);
        }
    }

    return 0;
}


// Create a new stack
Stack* createStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->size = 0;
    stack->capacity = 4;  // Initial capacity
    stack->array = (HashMap**)malloc(stack->capacity * sizeof(HashMap*));
    return stack;
}
// Free the memory allocated for the stack
void freeStack(Stack* stack) {
    if (stack) {
        free(stack->array);
        free(stack);
    }
}

// Push a value onto the stack
int stackPush(Stack* stack, HashMap* value) {
    // If stack is full, resize it
    if (stack->size == stack->capacity) {
        stackResize(stack, stack->capacity * 2);  // Double the capacity
    }
    stack->array[stack->size++] = value;
    return 0;
}

// Pop a value from the stack
int stackPop(Stack* stack, HashMap** value) {
    if (stackIsEmpty(stack)) {
        return -1; // Stack underflow
    }
    *value = stack->array[--stack->size];

    // If the stack is less than a quarter full, resize it to half the size
    if (stack->size <= stack->capacity / 4 && stack->capacity > 4) {
        stackResize(stack, stack->capacity / 2);
    }

    return 0;
}

// Peek at the top value of the stack without removing it
int stackPeek(Stack* stack, HashMap** value) {
    if (stackIsEmpty(stack)) {
        return -1; // Stack is empty
    }
    *value = stack->array[stack->size - 1];
    return 0;
}

// Check if the stack is empty
int stackIsEmpty(Stack* stack) {
    return stack->size == 0;
}

// Get the current size of the stack
size_t stackSize(Stack* stack) {
    return stack->size;
}

// Resize the stack's internal array to the new capacity
void stackResize(Stack* stack, size_t newCapacity) {
    stack->capacity = newCapacity;
    stack->array = (HashMap**)realloc(stack->array, stack->capacity * sizeof(HashMap*));
}

//add to the top of memory.c
#define HEAP_SIZE (1024 * 1024 * 16)
#include <stdio.h>

//add to memory.c, comment out original reallocate method
typedef struct FreeBlock {
    size_t size;
    struct FreeBlock* next;
} FreeBlock;

static uint8_t* heap = NULL;
static uint8_t* bumpPtr = NULL;         // for initial allocation
static uint8_t* heapEnd = NULL;
static FreeBlock* freeList = NULL;

void initHeap() {
    heap = malloc(HEAP_SIZE);
    bumpPtr = heap;
    heapEnd = heap + HEAP_SIZE;
    freeList = NULL;
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        // "Free" — add to free list
        if (pointer != NULL && oldSize > 0) {
            FreeBlock* block = (FreeBlock*)pointer;
            block->size = oldSize;
            block->next = freeList;
            freeList = block;
        }
        return NULL;
    }

    if (pointer == NULL) {
        // Fresh allocation — search free list first
        FreeBlock** curr = &freeList;
        while (*curr != NULL) {
            if ((*curr)->size >= newSize) {
                void* result = *curr;
                *curr = (*curr)->next;  // unlink from free list
                return result;
            }
            curr = &(*curr)->next;
        }
        // Nothing in free list, bump allocate
        if (bumpPtr + newSize > heapEnd) {
            fprintf(stderr, "Out of memory.\n");
            exit(1);
        }
        void* result = bumpPtr;
        bumpPtr += newSize;
        return result;
    }

    // Resize — allocate new, copy, free old
    void* newBlock = reallocate(NULL, 0, newSize);
    size_t copySize = oldSize < newSize ? oldSize : newSize;
    memcpy(newBlock, pointer, copySize);
    reallocate(pointer, oldSize, 0);
    return newBlock;
}

//add initHeap method declaration to memory.h
void initHeap();

//add to main.c
#include "memory.h"

//in main method
int main(int argc, const char* argv[]) {
    initHeap();
    initVM();
    //rest of method
}

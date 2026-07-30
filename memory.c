#include <stdlib.h>
#include <string.h>
#include "memory.h"

// Define our personal heap size (e.g., 4 Megabytes)
#define HEAP_SIZE (1024 * 1024 * 4)

// Block header structure stored inline right before every memory chunk
typedef struct BlockHeader {
    size_t size;               // Size of the data payload in bytes
    int is_free;               // 1 if free, 0 if allocated
    struct BlockHeader* next;  // Pointer to the next block in the heap sequence
} BlockHeader;

static void* personal_heap = NULL;
static BlockHeader* free_list_head = NULL;

// Initializes the personal heap once at startup
static void init_personal_heap() {
    if (personal_heap != NULL) return;
    
    personal_heap = malloc(HEAP_SIZE);
    if (personal_heap == NULL) {
        exit(1); // Fatal: system out of memory
    }

    // Set up the initial single giant free block spanning the entire heap
    free_list_head = (BlockHeader*)personal_heap;
    free_list_head->size = HEAP_SIZE - sizeof(BlockHeader);
    free_list_head->is_free = 1;
    free_list_head->next = NULL;
}

// Helper: Splits a free block if the leftover space is large enough
static void split_free_block(BlockHeader* current, size_t newSize) {
    size_t remaining_space = current->size - newSize;
    if (remaining_space > sizeof(BlockHeader) + 8) {
        BlockHeader* next_block = (BlockHeader*)((char*)current + sizeof(BlockHeader) + newSize);
        next_block->size = remaining_space - sizeof(BlockHeader);
        next_block->is_free = 1;
        next_block->next = current->next;

        current->size = newSize;
        current->next = next_block;
    }
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    // Ensure the personal heap is initialized
    if (personal_heap == NULL) {
        init_personal_heap();
    }

    // Case 1: Free allocation (newSize == 0)
    if (newSize == 0) {
        if (pointer == NULL) return NULL;
        
        // Locate header residing immediately before the user pointer
        BlockHeader* header = (BlockHeader*)((char*)pointer - sizeof(BlockHeader));
        header->is_free = 1;

        // Coalesce adjacent free blocks sequentially to combat fragmentation
        BlockHeader* current = free_list_head;
        while (current != NULL && current->next != NULL) {
            if (current->is_free && current->next->is_free) {
                current->size += sizeof(BlockHeader) + current->next->size;
                current->next = current->next->next;
            } else {
                current = current->next;
            }
        }
        return NULL;
    }

    // Case 2: Allocate new block (pointer == NULL, oldSize == 0)
    if (pointer == NULL) {
        BlockHeader* current = free_list_head;
        while (current != NULL) {
            if (current->is_free && current->size >= newSize) {
                split_free_block(current, newSize);
                current->is_free = 0;
                return (char*)current + sizeof(BlockHeader);
            }
            current = current->next;
        }
        // Heap exhaustion error if no suitable block found
        exit(1);
    }

    // Case 3: Resize existing allocation (pointer != NULL)
    BlockHeader* header = (BlockHeader*)((char*)pointer - sizeof(BlockHeader));
    
    // If new size fits within the current block (shrink or keep same)
    if (header->size >= newSize) {
        split_free_block(header, newSize);
        return pointer;
    }

    // Otherwise (grow), allocate a new block, copy old content, and free the old block
    void* new_pointer = reallocate(NULL, 0, newSize);
    if (new_pointer == NULL) return NULL;

    size_t copy_size = oldSize < header->size ? oldSize : header->size;
    memcpy(new_pointer, pointer, copy_size);
    reallocate(pointer, oldSize, 0);

    return new_pointer;
}
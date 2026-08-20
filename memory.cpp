#include "memory.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "object.hpp"
#include "vm.hpp"

Heap& getGlobalHeap() {
    static Heap heap;
    return heap;
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    return getGlobalHeap().reallocate(pointer, oldSize, newSize);
}

Heap::Heap() {
    personalHeap = std::malloc(HEAP_SIZE);
    if (!personalHeap) {
        std::cerr << "Fatal: Could not allocate personal heap (" << HEAP_SIZE << " bytes).\n";
        std::exit(1);
    }

    freeListHead = static_cast<BlockHeader*>(personalHeap);
    freeListHead->size = HEAP_SIZE - sizeof(BlockHeader);
    freeListHead->isFree = true;
    freeListHead->next = nullptr;
}

Heap::~Heap() {
    std::free(personalHeap);
    personalHeap = nullptr;
    freeListHead = nullptr;
}

void Heap::splitFreeBlock(BlockHeader* current, size_t newSize) {
    size_t remainingSpace = current->size - newSize;
    if (remainingSpace > sizeof(BlockHeader) + 8) {
        auto* nextBlock = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(current) + sizeof(BlockHeader) + newSize
        );
        nextBlock->size = remainingSpace - sizeof(BlockHeader);
        nextBlock->isFree = true;
        nextBlock->next = current->next;

        current->size = newSize;
        current->next = nextBlock;
    }
}

void Heap::coalesce() {
    BlockHeader* current = freeListHead;
    while (current != nullptr && current->next != nullptr) {
        if (current->isFree && current->next->isFree) {
            current->size += sizeof(BlockHeader) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void* Heap::reallocate(void* pointer, size_t oldSize, size_t newSize) {
    // Case 1: Free allocation (newSize == 0)
    if (newSize == 0) {
        if (pointer == nullptr) return nullptr;

        auto* header = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(pointer) - sizeof(BlockHeader)
        );
        header->isFree = true;
        coalesce();
        return nullptr;
    }

    // Case 2: Allocate new block (pointer == nullptr, oldSize == 0)
    if (pointer == nullptr) {
        BlockHeader* current = freeListHead;
        while (current != nullptr) {
            if (current->isFree && current->size >= newSize) {
                splitFreeBlock(current, newSize);
                current->isFree = false;
                return reinterpret_cast<char*>(current) + sizeof(BlockHeader);
            }
            current = current->next;
        }

        std::cerr << "Fatal: Heap exhausted. Out of memory.\n";
        std::exit(1);
    }

    // Case 3: Resize existing allocation (pointer != nullptr)
    auto* header = reinterpret_cast<BlockHeader*>(
        reinterpret_cast<char*>(pointer) - sizeof(BlockHeader)
    );

    if (header->size >= newSize) {
        splitFreeBlock(header, newSize);
        return pointer;
    }

    // Growth: allocate new block, transfer data, free old block
    void* newPointer = reallocate(nullptr, 0, newSize);
    if (!newPointer) return nullptr;

    size_t copySize = std::min(oldSize, header->size);
    std::memcpy(newPointer, pointer, copySize);
    reallocate(pointer, oldSize, 0);

    return newPointer;
}

void freeObject(Obj* object) {
    switch (object->type) {
        case ObjType::OBJ_STRING: {
            auto* string = reinterpret_cast<ObjString*>(object);
            reallocate(string, sizeof(ObjString) + string->length + 1, 0);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = VM::getInstance().getObjects();
    while (object != nullptr) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}
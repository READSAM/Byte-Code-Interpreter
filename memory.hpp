#pragma once

#include <cstddef>
#include <cstdint>
#include "common.hpp"

struct Obj;

class Heap {
public:
    static constexpr size_t HEAP_SIZE = 1024 * 1024 * 4; // 4 MB

    Heap();
    ~Heap();

    // Prevent copies
    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;

    void* reallocate(void* pointer, size_t oldSize, size_t newSize);

private:
    struct BlockHeader {
        size_t size;            // size of payload in bytes
        bool isFree;            // true if free
        BlockHeader* next;      // Pointer to the next
    };

    void* personalHeap = nullptr;
    BlockHeader* freeListHead = nullptr;

    void splitFreeBlock(BlockHeader* current, size_t newSize);
    void coalesce();
};

// Global memory allocation instance accessor
Heap& getGlobalHeap();

// Global allocator wrapper
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

// Type-safe templated memory helpers replacing C macros
template <typename T>
T* allocate(size_t count = 1) {
    return static_cast<T*>(reallocate(nullptr, 0, sizeof(T) * count));
}

template <typename T>
void freeItem(T* pointer) {
    reallocate(pointer, sizeof(T), 0);
}

template <typename T>
T* growArray(T* pointer, size_t oldCount, size_t newCount) {
    return static_cast<T*>(reallocate(pointer, sizeof(T) * oldCount, sizeof(T) * newCount));
}

template <typename T>
void freeArray(T* pointer, size_t oldCount) {
    reallocate(pointer, sizeof(T) * oldCount, 0);
}

constexpr size_t growCapacity(size_t capacity) noexcept {
    return capacity < 8 ? 8 : capacity * 2;
}

void freeObject(Obj* object);
void freeObjects();
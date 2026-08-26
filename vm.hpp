#pragma once

#include <array>

#include <string_view>

#include "common.hpp"
#include "chunk.hpp"
#include "object.hpp"
#include "value.hpp"
#include "table.hpp"

enum class InterpretResult : uint8_t {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class VM {
public:
    static constexpr size_t STACK_MAX = 256;

    VM();
    ~VM();

    // disable copy semantics
    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;

    //singleton access for memory allocators and global helpers
    static VM& getInstance() noexcept { return *instance; }

    InterpretResult interpret(std::string_view source);

    void push(Value value);
    Value pop();

    // heap tracking accessors for GC / allocator
    Obj* getObjects() const noexcept { return objects; }
    void setObjects(Obj* objectList) noexcept { objects = objectList; }

public:
    static VM* instance;

    Chunk* chunk = nullptr;
    const uint8_t* ip = nullptr;

    std::array<Value, STACK_MAX> stack{};
    Value* stackTop = nullptr;
    Obj* objects = nullptr;
    Table strings;

    void resetStack();
    void runtimeError(const char* format, ...);
    Value peek(int distance = 0) const;
    bool isFalsey(Value value) const noexcept;
    void concatenate();
    InterpretResult run();
};
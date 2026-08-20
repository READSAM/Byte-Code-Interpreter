#pragma once

#include <cstdint>
#include <vector>
#include "common.hpp"
#include "value.hpp"

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN,
};

struct RleForm {
    int lineNum;
    int count;
};

class Chunk {
public:
    Chunk() = default;
    ~Chunk() = default;

    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) noexcept = default;
    Chunk& operator=(Chunk&&) noexcept = default;

    void write(uint8_t byte, int line);
    void write(OpCode opcode, int line);
    void writeConstant(Value value, int line);
    int addConstant(Value value);
    int getLine(int instructionOffset) const;

    const std::vector<uint8_t>& getCode() const noexcept { return code; }
    const std::vector<Value>& getConstants() const noexcept { return constants; }
    std::size_t count() const noexcept { return code.size(); }

private:
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<RleForm> lines;
};
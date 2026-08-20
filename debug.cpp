#include "debug.hpp"

#include <cstdio>
#include <string_view>
#include "value.hpp"

static int simpleInstruction(std::string_view name, int offset) {
    std::printf("%s\n", name.data());
    return offset + 1;
}

static int constantInstruction(std::string_view name, const Chunk& chunk, int offset) {
    uint8_t constantIndex = chunk.getCode()[offset + 1];
    std::printf("%-16s %4d '", name.data(), constantIndex);
    printValue(chunk.getConstants()[constantIndex]);
    std::printf("'\n");
    return offset + 2;
}

static int longConstantInstruction(std::string_view name, const Chunk& chunk, int offset) {
    const auto& code = chunk.getCode();
    int constantIndex = (static_cast<int>(code[offset + 1]) << 16) |
                        (static_cast<int>(code[offset + 2]) << 8)  |
                        (static_cast<int>(code[offset + 3]));

    std::printf("%-16s %4d '", name.data(), constantIndex);
    printValue(chunk.getConstants()[constantIndex]);
    std::printf("'\n");
    return offset + 4;
}

void disassembleChunk(const Chunk& chunk, std::string_view name) {
    std::printf("== %s ==\n", name.data());

    for (int offset = 0; offset < static_cast<int>(chunk.count());) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(const Chunk& chunk, int offset) {
    std::printf("%04d ", offset);

    int line = chunk.getLine(offset);
    if (offset > 0 && line == chunk.getLine(offset - 1)) {
        std::printf("   | ");
    } else {
        std::printf("%4d ", line);
    }

    uint8_t instruction = chunk.getCode()[offset];
    auto op = static_cast<OpCode>(instruction);

    switch (op) {
        case OpCode::OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OpCode::OP_CONSTANT_LONG:
            return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);
        case OpCode::OP_NIL:
            return simpleInstruction("OP_NIL", offset);
        case OpCode::OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OpCode::OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OpCode::OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);
        case OpCode::OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);
        case OpCode::OP_LESS:
            return simpleInstruction("OP_LESS", offset);
        case OpCode::OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OpCode::OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        case OpCode::OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OpCode::OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OpCode::OP_NOT:
            return simpleInstruction("OP_NOT", offset);
        case OpCode::OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OpCode::OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            std::printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
#include "chunk.hpp"

void Chunk::write(uint8_t byte, int line) {
    code.push_back(byte);

    if (!lines.empty() && lines.back().lineNum == line) {
        lines.back().count++;
    } else {
        lines.push_back({line, 1});
    }
}

void Chunk::write(OpCode opcode, int line) {
    write(static_cast<uint8_t>(opcode), line);
}

int Chunk::addConstant(Value value) {
    constants.push_back(value);
    return static_cast<int>(constants.size() - 1);
}

void Chunk::writeConstant(Value value, int line) {
    int constantIndex = addConstant(value);

    if (constantIndex <= 255) {
        write(OpCode::OP_CONSTANT, line);
        write(static_cast<uint8_t>(constantIndex), line);
    } else {
        write(OpCode::OP_CONSTANT_LONG, line);
        write(static_cast<uint8_t>((constantIndex >> 16) & 0xFF), line);
        write(static_cast<uint8_t>((constantIndex >> 8) & 0xFF), line);
        write(static_cast<uint8_t>(constantIndex & 0xFF), line);
    }
}

int Chunk::getLine(int instructionOffset) const {
    int currentOffset = 0;
    for (const auto& entry : lines) {
        currentOffset += entry.count;
        if (instructionOffset < currentOffset) {
            return entry.lineNum;
        }
    }
    return -1;
}
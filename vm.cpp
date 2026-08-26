#include "vm.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "compiler.hpp"
#include "debug.hpp"
#include "memory.hpp"
#include "table.hpp"

VM* VM::instance = nullptr;

VM::VM() {
    instance = this;
    globals.init();
    strings.init();
    resetStack();
}

VM::~VM() {
    globals.free();
    strings.free();
    freeObjects();
    instance = nullptr;
}

void VM::resetStack() {
    stackTop = stack.data();
}

void VM::push(Value value) {
    *stackTop = value;
    stackTop++;
}

Value VM::pop() {
    stackTop--;
    return *stackTop;
}

Value VM::peek(int distance) const {
    return stackTop[-1 - distance];
}

bool VM::isFalsey(Value value) const noexcept {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
    std::fputs("\n", stderr);

    size_t instructionOffset = ip - chunk->getCode().data() - 1;
    int line = chunk->getLine(static_cast<int>(instructionOffset));
    std::fprintf(stderr, "[line %d] in script\n", line);

    resetStack();
}

void VM::concatenate() {
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    int length = a->length + b->length;
    char* chars = allocate<char>(length + 1);
    std::memcpy(chars, a->chars, a->length);
    std::memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    freeArray(chars, length + 1);
    push(OBJ_VAL(result));
}

InterpretResult VM::run() {
    auto readByte = [this]() -> uint8_t {
        return *ip++;
    };

    auto readConstant = [this, &readByte]() -> Value {
        return chunk->getConstants()[readByte()];
    };

    auto readString=[this,&readConstant]()-> ObjString*{ 
        return AS_STRING(readConstant());
        };

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        std::printf("          ");
        for (Value* slot = stack.data(); slot < stackTop; slot++) {
            std::printf("[ ");
            printValue(*slot);
            std::printf(" ]");
        }
        std::printf("\n");
        disassembleInstruction(*chunk, static_cast<int>(ip - chunk->getCode().data()));
#endif

        uint8_t instruction = readByte();
        switch (static_cast<OpCode>(instruction)) {
            case OpCode::OP_CONSTANT: {
                Value constant = readConstant();
                push(constant);
                break;
            }
            case OpCode::OP_NIL:   push(NIL_VAL()); break;
            case OpCode::OP_TRUE:  push(BOOL_VAL(true)); break;
            case OpCode::OP_FALSE: push(BOOL_VAL(false)); break;
            case OpCode::OP_POP: pop(); break;
            case OpCode::OP_GET_GLOBAL:{
                ObjString* name=readString();
                Value value;
                if(!globals.get(name,&value))
                {
                    runtimeError("Undefined variable '%s'.",name->chars);
                    return InterpretResult::RUNTIME_ERROR;
                }

                push(value);
                break;
            }
            case OpCode:: OP_DEFINE_GLOBAL:{
                ObjString* name= readString();
                globals.set(name,peek(0));
                pop();
                break;
            }
            case OpCode::OP_SET_GLOBAL:{
                ObjString* name= readString();
                if(globals.set(name,peek(0)))
                {
                    globals.remove(name);
                    runtimeError("Undefine variable '%s' .",name->chars);
                    return InterpretResult::RUNTIME_ERROR;
                }

                break;
            }
            case OpCode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OpCode::OP_GREATER: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(BOOL_VAL(a > b));
                break;
            }
            case OpCode::OP_LESS: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(BOOL_VAL(a < b));
                break;
            }
            case OpCode::OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::OP_SUBTRACT: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a - b));
                break;
            }
            case OpCode::OP_MULTIPLY: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a * b));
                break;
            }
            case OpCode::OP_DIVIDE: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a / b));
                break;
            }
            case OpCode::OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OpCode::OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OpCode::OP_PRINT:
                printValue(pop());
                printf("\n");
                break;
            case OpCode::OP_RETURN: {
                //exit 
                return InterpretResult::OK;
            }
            default:
                runtimeError("Unknown opcode: %d", instruction);
                return InterpretResult::RUNTIME_ERROR;
        }
    }
}

InterpretResult VM::interpret(std::string_view source) {
    Chunk compiledChunk;

    if (!Compiler{}.compile(source, compiledChunk)) {
        return InterpretResult::COMPILE_ERROR;
    }

    this->chunk = &compiledChunk;
    this->ip = chunk->getCode().data();

    return run();
}

InterpretResult interpret(const char* source) {
    return VM::getInstance().interpret(source);
}

void push(Value value) {
    VM::getInstance().push(value);
}

Value pop() {
    return VM::getInstance().pop();
}
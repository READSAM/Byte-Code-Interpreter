#include "value.hpp"

#include <cstdio>
#include <cstring>
#include "object.hpp"

void printValue(Value value) {
    switch (value.type) {
        case ValueType::VAL_BOOL:
            std::printf(AS_BOOL(value) ? "true" : "false");
            break;
        case ValueType::VAL_NIL:
            std::printf("nil");
            break;
        case ValueType::VAL_NUMBER:
            std::printf("%g", AS_NUMBER(value));
            break;
        case ValueType::VAL_OBJ:
            printObject(value);
            break;
    }
}

bool valuesEqual(Value a, Value b) noexcept {
    if (a.type != b.type) return false;

    switch (a.type) {
        case ValueType::VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case ValueType::VAL_NIL:
            return true;
        case ValueType::VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case ValueType::VAL_OBJ:
            return AS_OBJ(a)==AS_OBJ(b);
}

return false;
}
bool Value::operator==(const Value& other) const noexcept {
    return valuesEqual(*this, other);
}
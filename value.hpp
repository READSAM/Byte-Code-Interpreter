#pragma once

#include <cstdint>
#include <vector>
#include "common.hpp"

// Forward declarations
struct Obj;
struct ObjString;

enum class ValueType : uint8_t {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
};

struct Value {
    ValueType type{ValueType::VAL_NIL};
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as{ .number = 0.0 };

    constexpr Value() noexcept = default;
    constexpr explicit Value(bool b) noexcept : type(ValueType::VAL_BOOL) { as.boolean = b; }
    constexpr explicit Value(double n) noexcept : type(ValueType::VAL_NUMBER) { as.number = n; }
    constexpr explicit Value(Obj* o) noexcept : type(ValueType::VAL_OBJ) { as.obj = o; }

    bool operator==(const Value& other) const noexcept;
    bool operator!=(const Value& other) const noexcept { return !(*this == other); }
};

//type checkers
constexpr bool IS_BOOL(Value value) noexcept { return value.type == ValueType::VAL_BOOL; }
constexpr bool IS_NIL(Value value) noexcept { return value.type == ValueType::VAL_NIL; }
constexpr bool IS_NUMBER(Value value) noexcept { return value.type == ValueType::VAL_NUMBER; }
constexpr bool IS_OBJ(Value value) noexcept { return value.type == ValueType::VAL_OBJ; }

// value extractors
constexpr bool AS_BOOL(Value value) noexcept { return value.as.boolean; }
constexpr double AS_NUMBER(Value value) noexcept { return value.as.number; }
constexpr Obj* AS_OBJ(Value value) noexcept { return value.as.obj; }

// value constructors
constexpr Value BOOL_VAL(bool value) noexcept { return Value(value); }
constexpr Value NIL_VAL() noexcept { return Value(); }
constexpr Value NUMBER_VAL(double value) noexcept { return Value(value); }
constexpr Value OBJ_VAL(Obj* value) noexcept { return Value(value); }

using ValueArray = std::vector<Value>;

bool valuesEqual(Value a, Value b) noexcept;
void printValue(Value value);
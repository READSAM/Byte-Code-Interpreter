#pragma once

#include <string_view>
#include "common.hpp"
#include "value.hpp"

enum class ObjType : uint8_t {
    OBJ_STRING,
};

struct Obj {
    ObjType type;
    Obj* next{nullptr};
};

struct ObjString : public Obj {
    int length{0};
    uint32_t hash{0};
    char chars[1]{'\0'};
    
};

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
ObjString* copyString(std::string_view str);

void printObject(Value value);

inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

inline bool isString(Value value) {
    return isObjType(value, ObjType::OBJ_STRING);
}

inline ObjString* asString(Value value) {
    return static_cast<ObjString*>(AS_OBJ(value));
}

inline const char* asCString(Value value) {
    return static_cast<ObjString*>(AS_OBJ(value))->chars;
}

#define OBJ_TYPE(value)   (AS_OBJ(value)->type)
#define IS_STRING(value)  isString(value)
#define AS_STRING(value)  asString(value)
#define AS_CSTRING(value) asCString(value)
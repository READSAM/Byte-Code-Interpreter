#include "object.hpp"

#include <cstdio>
#include <cstring>
#include "memory.hpp"
#include "vm.hpp"

static Obj* allocateObject(size_t size, ObjType type) {
    auto* object = static_cast<Obj*>(reallocate(nullptr, 0, size));
    object->type = type;

    // Track object in VM garbage collection / lifetime linked list
    object->next = VM::getInstance().getObjects();
    VM::getInstance().setObjects(object);

    return object;
}

static ObjString* allocateString(const char* chars, int length) {

    // total allocation size = struct size + string length(including'\0')
    // we subtract 1 because sizeof(ObjString) already includes chars[1].
    size_t totalSize = sizeof(ObjString) + length;
    
    auto* string = reinterpret_cast<ObjString*>(allocateObject(totalSize, ObjType::OBJ_STRING));
    string->length = length;
    
    std::memcpy(string->chars, chars, length);
    string->chars[length] = '\0';

    return string;
}

ObjString* takeString(char* chars, int length) {
    // in single-block design, takeString copies into the combined ObjString block
    return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length) {
    return allocateString(chars, length);
}

ObjString* copyString(std::string_view str) {
    return allocateString(str.data(), static_cast<int>(str.length()));
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case ObjType::OBJ_STRING:
            std::printf("%s", AS_CSTRING(value));
            break;
    }
}
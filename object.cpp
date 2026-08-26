#include "object.hpp"

#include <cstdio>
#include <cstring>

#include "memory.hpp"
#include "vm.hpp"
#include "table.hpp"

static Obj* allocateObject(size_t size, ObjType type) {
    auto* object = static_cast<Obj*>(reallocate(nullptr, 0, size));
    object->type = type;

    // Track object in VM garbage collection / lifetime linked list
    object->next = VM::getInstance().getObjects();
    VM::getInstance().setObjects(object);

    return object;
}

static ObjString* allocateString(const char* chars, int length,uint32_t hash) {

    // total allocation size = struct size + string length(including'\0')
    // we subtract 1 because sizeof(ObjString) already includes chars[1].
    size_t totalSize = sizeof(ObjString) + length;
    
    auto* string = reinterpret_cast<ObjString*>(allocateObject(totalSize, ObjType::OBJ_STRING));
    string->length = length;
    
    std::memcpy(string->chars, chars, length);
    string->chars[length] = '\0';
    string->hash=hash;
    VM::getInstance().strings.set( string, NIL_VAL());

    return string;
}

static uint32_t hashString(const char* key, int length)
{
    uint32_t hash=2166136261u;
    for(int i=0;i<length;i++)
    {
        hash^=(uint8_t)key[i];
        hash*=16777619;
    }

    return hash;
}

ObjString* takeString(char* chars, int length) {
    // in single-block design, takeString copies into the combined ObjString block
    uint32_t hash=hashString(chars,length);
    return allocateString(chars, length,hash);
}

ObjString* copyString(const char* chars, int length) {
    uint32_t hash=hashString(chars,length);
    ObjString* interned = VM::getInstance().strings.findString(chars, length, hash);

    if(interned !=NULL){
        freeArray(const_cast<char*>(chars), length+1);
        return interned;
    }
    return allocateString(chars, length,hash);
}


void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case ObjType::OBJ_STRING:
            std::printf("%s", AS_CSTRING(value));
            break;
    }
}
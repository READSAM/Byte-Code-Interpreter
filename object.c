#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

// static Obj* allocateObject(size_t size, ObjType type) {
//   Obj* object = (Obj*)reallocate(NULL, 0, size); //create Obj of size ObjString type;
//   object->type = type; 

//   object->next = vm.objects;
//   vm.objects = object;
//   return object;
// }

static ObjString* allocateString(char* chars, int length) { //got the character array and its length
  ObjString* string = (ObjString*) reallocate(NULL,0,sizeof(ObjString)+length+1);
  //ALLOCATE_OBJ(ObjString, OBJ_STRING); //remember that ObjString is ingerently an Obj first , so first create Obj first
  string->obj.type = OBJ_STRING;
  string->length = length; 
 // string->chars = chars;
  memcpy(string->chars, chars, length);
  string->chars[length] = '\0';

  return string; //return ObjString object;
}

ObjString* takeString(char* chars, int length) {
  return allocateString(chars, length);
}


//first copy the string from the source code to the heap 
ObjString* copyString(const char* chars, int length) {
  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length); //now moving to creation of string object
}

void printObject(Value value)
{
    switch(OBJ_TYPE(value))
    {
        case OBJ_STRING: 
            printf("%s",AS_CSTRING(value));
            break;
    }
}
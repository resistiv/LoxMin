#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

static Object* AllocateObject(size_t size, ObjectType type);
static ObjectString* AllocateString(char* chars, int length);

/**
 * @brief Allocates an object of a given type.
 */
#define ALLOCATE_OBJECT(type, objectType) \
        (type*)AllocateObject(sizeof(type), objectType)

ObjectString* TakeString(char* chars, int length)
{
    return AllocateString(chars, length);
}

ObjectString* CopyString(const char* chars, int length)
{
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return AllocateString(heapChars, length);
}

void PrintObject(Value value)
{
    switch(OBJECT_TYPE(value))
    {
        case OBJECT_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}

/**
 * @brief Creates and allocates an Object.
 * 
 * @param size The size of the Object.
 * @param type The ObjectType of the Object.
 * @return Object* A pointer to the resulting Object.
 */
static Object* AllocateObject(size_t size, ObjectType type)
{
    Object* object = (Object*)Reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;

    return object;
}

/**
 * @brief Creates and allocates an ObjectString object.
 * 
 * @param chars A pointer to the characters belonging to the object.
 * @param length The length of the string.
 * @return ObjectString* A pointer to the resulting ObjectString.
 */
static ObjectString* AllocateString(char* chars, int length)
{
    ObjectString* string = ALLOCATE_OBJECT(ObjectString, OBJECT_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}

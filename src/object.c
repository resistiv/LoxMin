#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static Object* AllocateObject(size_t size, ObjectType type);
static ObjectString* AllocateString(char* chars, int length, uint32_t hash);
static uint32_t HashString(const char* key, int length);

/**
 * @brief Allocates an object of a given type.
 */
#define ALLOCATE_OBJECT(type, objectType) \
        (type*)AllocateObject(sizeof(type), objectType)

ObjectString* TakeString(char* chars, int length)
{
    // Check if an equivalent string is already present in memory
    uint32_t hash = HashString(chars, length);
    ObjectString* interned = TableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
    {
        // Get rid of the previous string
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return AllocateString(chars, length, hash);
}

ObjectString* CopyString(const char* chars, int length)
{
    // Check if an equivalent string is already present in memory
    uint32_t hash = HashString(chars, length);
    ObjectString* interned = TableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
    {
        return interned;
    }

    char* heapChars = ALLOCATE(char, length + 1);
    
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return AllocateString(heapChars, length, hash);
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
static ObjectString* AllocateString(char* chars, int length, uint32_t hash)
{
    ObjectString* string = ALLOCATE_OBJECT(ObjectString, OBJECT_STRING);

    string->length = length;
    string->chars = chars;
    string->hash = hash;
    TableSet(&vm.strings, string, NIL_VALUE);

    return string;
}

/**
 * @brief Hashes a string.
 * 
 * @param key A string to hash.
 * @param length The length of the string.
 * @return uint32_t 
 */
static uint32_t HashString(const char* key, int length)
{
    uint32_t hash = 2166136261U;
    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

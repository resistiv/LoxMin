#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static void PrintFunction(ObjectFunction* function);
static Object* AllocateObject(size_t size, ObjectType type);
static ObjectString* AllocateString(char* chars, int length, uint32_t hash);
static uint32_t HashString(const char* key, int length);

/**
 * @brief Allocates an object of a given type.
 */
#define ALLOCATE_OBJECT(type, objectType) \
        (type*)AllocateObject(sizeof(type), objectType)

ObjectBoundMethod* NewBoundMethod(Value receiver, ObjectClosure* method)
{
    ObjectBoundMethod* bound = ALLOCATE_OBJECT(ObjectBoundMethod, OBJECT_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjectInstance* NewInstance(ObjectClass* _class)
{
    ObjectInstance* instance = ALLOCATE_OBJECT(ObjectInstance, OBJECT_INSTANCE);
    instance->_class = _class;
    InitTable(&instance->fields);
    return instance;
}

ObjectClass* NewClass(ObjectString* name)
{
    ObjectClass* _class = ALLOCATE_OBJECT(ObjectClass, OBJECT_CLASS);
    _class->name = name;
    InitTable(&_class->methods);
    return _class;
}

ObjectUpvalue* NewUpvalue(Value* slot)
{
    ObjectUpvalue* upvalue = ALLOCATE_OBJECT(ObjectUpvalue, OBJECT_UPVALUE);
    upvalue->closed = NIL_VALUE;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

ObjectClosure* NewClosure(ObjectFunction* function)
{
    ObjectUpvalue** upvalues = ALLOCATE(ObjectUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++)
    {
        upvalues[i] = NULL;
    }

    ObjectClosure* closure = ALLOCATE_OBJECT(ObjectClosure, OBJECT_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjectFunction* NewFunction()
{
    ObjectFunction* function = ALLOCATE_OBJECT(ObjectFunction, OBJECT_FUNCTION);
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    InitChunk(&function->chunk);
    return function;
}

ObjectNative* NewNative(NativeFn function)
{
    ObjectNative* native = ALLOCATE_OBJECT(ObjectNative, OBJECT_NATIVE);
    native->function = function;
    return native;
}

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
        case OBJECT_BOUND_METHOD:
            PrintFunction(AS_BOUND_METHOD(value)->method->function);
            break;
        case OBJECT_CLASS:
            printf("%s", AS_CLASS(value)->name->chars);
            break;
        case OBJECT_INSTANCE:
            printf("%s instance", AS_INSTANCE(value)->_class->name->chars);
            break;
        case OBJECT_UPVALUE:
            printf("upvalue");
            break;
        case OBJECT_CLOSURE:
            PrintFunction(AS_CLOSURE(value)->function);
            break;
        case OBJECT_FUNCTION:
            PrintFunction(AS_FUNCTION(value));
            break;
        case OBJECT_NATIVE:
            printf("<native fn>");
            break;
        case OBJECT_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}

/**
 * @brief Prints a function object.
 * 
 * @param function An ObjectFunction to print.
 */
static void PrintFunction(ObjectFunction* function)
{
    if (function->name == NULL)
    {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
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
    object->isMarked = false;
    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

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

    StackPush(OBJECT_VALUE(string));
    TableSet(&vm.strings, string, NIL_VALUE);
    StackPop();

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

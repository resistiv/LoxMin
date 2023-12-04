#ifndef loxmin_object_h
#define loxmin_object_h

#include "common.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

/**
 * @brief Enumerates all available Object types.
 */
typedef enum
{
    OBJECT_CLASS,
    OBJECT_INSTANCE,
    OBJECT_UPVALUE,
    OBJECT_CLOSURE,
    OBJECT_FUNCTION,
    OBJECT_NATIVE,
    OBJECT_STRING,
} ObjectType;

/**
 * @brief Represents an object.
 */
struct Object 
{
    ObjectType type;
    bool isMarked;
    struct Object* next;
};

/**
 * @brief Represents a string.
 */
struct ObjectString
{
    Object obj;
    int length;
    char* chars;
    uint32_t hash;
};

/**
 * @brief Represents a function.
 */
typedef struct
{
    Object obj;
    int arity;
    int upvalueCount;
    Chunk chunk;
    ObjectString* name;
} ObjectFunction;

/**
 * @brief Represents an upvalue.
 */
typedef struct ObjectUpvalue
{
    Object obj;
    Value* location;
    Value closed;
    struct ObjectUpvalue* next;
} ObjectUpvalue;

/**
 * @brief Represents a closure.
 */
typedef struct 
{
    Object obj;
    ObjectFunction* function;
    ObjectUpvalue** upvalues;
    int upvalueCount;
} ObjectClosure;

typedef Value (*NativeFn)(int argCount, Value* args);

/**
 * @brief Represents a native function.
 */
typedef struct
{
    Object obj;
    NativeFn function;
} ObjectNative;

/**
 * @brief Represents a class.
 */
typedef struct
{
    Object obj;
    ObjectString* name;
    Table methods;
} ObjectClass;

/**
 * @brief Represents an instance.
 */
typedef struct
{
    Object obj;
    ObjectClass* _class;
    Table fields;
} ObjectInstance;

#define OBJECT_TYPE(value)  (AS_OBJECT(value)->type)

#define IS_CLASS(value)     IsObjectType(value, OBJECT_CLASS)
#define IS_CLOSURE(value)   IsObjectType(value, OBJECT_CLOSURE)
#define IS_FUNCTION(value)  IsObjectType(value, OBJECT_FUNCTION)
#define IS_INSTANCE(value)  IsObjectType(value, OBJECT_INSTANCE)
#define IS_NATIVE(value)    IsObjectType(value, OBJECT_NATIVE)
#define IS_STRING(value)    IsObjectType(value, OBJECT_STRING)

#define AS_CLASS(value)     ((ObjectClass*)AS_OBJECT(value))
#define AS_CLOSURE(value)   ((ObjectClosure*)AS_OBJECT(value))
#define AS_FUNCTION(value)  ((ObjectFunction*)AS_OBJECT(value))
#define AS_INSTANCE(value)  ((ObjectInstance*)AS_OBJECT(value))
#define AS_NATIVE(value)    (((ObjectNative*)AS_OBJECT(value))->function)
#define AS_STRING(value)    ((ObjectString*)AS_OBJECT(value))
#define AS_CSTRING(value)   (((ObjectString*)AS_OBJECT(value))->chars)

/**
 * @brief Instantiates a new class instance.
 * 
 * @param _class An ObjectClass to produce an instance of.
 * @return ObjectInstance* A pointer to the new ObjectInstance object.
 */
ObjectInstance* NewInstance(ObjectClass* _class);

/**
 * @brief Instantiates a new class.
 * 
 * @param name The name of the class.
 * @return ObjectClass* A pointer to the new ObjectClass object.
 */
ObjectClass* NewClass(ObjectString* name);

/**
 * @brief Instantiates a new upvalue.
 * 
 * @param slot The slot of the upvalue.
 * @return ObjectUpvalue* A pointer to the new ObjectUpvalue object.
 */
ObjectUpvalue* NewUpvalue(Value* slot);

/**
 * @brief Instantiates a new closure.
 * 
 * @param function A function to close around.
 * @return ObjectClosure* A pointer to the new ObjectClosure object.
 */
ObjectClosure* NewClosure(ObjectFunction* function);

/**
 * @brief Instantiates a new function.
 * 
 * @return ObjectFunction* A pointer to the new ObjectFunction object.
 */
ObjectFunction* NewFunction();

/**
 * @brief Instantiates a new native function.
 * 
 * @param function A native function.
 * @return ObjectNative* A pointer to the new ObjectNative object.
 */
ObjectNative* NewNative(NativeFn function);

/**
 * @brief Takes ownership of a given string and allocates it.
 * 
 * @param chars A pointer to the string's characters.
 * @param length The length of the string.
 * @return ObjectString* A pointer to the resulting ObjectString.
 */
ObjectString* TakeString(char* chars, int length);

/**
 * @brief Copies a string into memory and creates an ObjectString object.
 * 
 * @param chars A pointer to the characters to copy.
 * @param length The length of the string.
 * @return ObjectString* A pointer to the resulting ObjectString.
 */
ObjectString* CopyString(const char* chars, int length);

/**
 * @brief Prints an Object and its contents in a human-readable form.
 * 
 * @param value A Value holding an Object.
 */
void PrintObject(Value value);

/**
 * @brief Checks if a Value is an Object and its type matches a given ObjectType.
 * 
 * @param value A Value to check.
 * @param type An ObjectType to verify.
 * @return true If the Value is an Object and its type matches the specified ObjectType.
 * @return false If the Value is not Object or its type does not match the specified ObjectType.
 */
static inline bool IsObjectType(Value value, ObjectType type)
{
    return IS_OBJECT(value) && OBJECT_TYPE(value) == type;
}

#endif

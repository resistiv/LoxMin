#ifndef loxmin_object_h
#define loxmin_object_h

#include "common.h"
#include "value.h"

#define OBJECT_TYPE(value)  (AS_OBJECT(value)->type)

#define IS_STRING(value)    IsObjectType(value, OBJECT_STRING)

#define AS_STRING(value)    ((ObjectString*)AS_OBJECT(value))
#define AS_CSTRING(value)   (((ObjectString*)AS_OBJECT(value))->chars)

/**
 * @brief Enumerates all available Obj types.
 */
typedef enum
{
    OBJECT_STRING,
} ObjectType;

/**
 * @brief Represents an object.
 */
struct Object 
{
    ObjectType type;
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
};

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
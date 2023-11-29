#ifndef loxmin_value_h
#define loxmin_value_h

#include "common.h"

typedef struct Object Object;
typedef struct ObjectString ObjectString;

/**
 * @brief Enumerates all available Value types.
 */
typedef enum
{
    VALUE_BOOL,
    VALUE_NIL,
    VALUE_NUMBER,
    VALUE_OBJECT,
} ValueType;

/**
 * @brief Represents a value.
 */
typedef struct 
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        Object* obj;
    } as;
} Value;

#define IS_BOOL(value)   ((value).type == VALUE_BOOL)
#define IS_NIL(value)    ((value).type == VALUE_NIL)
#define IS_NUMBER(value) ((value).type == VALUE_NUMBER)
#define IS_OBJECT(value) ((value).type == VALUE_OBJECT)

#define AS_BOOL(value)   ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJECT(value) ((value).as.obj)

#define BOOL_VALUE(value)    ((Value){VALUE_BOOL,   {.boolean = value}})
#define NIL_VALUE            ((Value){VALUE_NIL,    {.number = 0}})
#define NUMBER_VALUE(value)  ((Value){VALUE_NUMBER, {.number = value}})
#define OBJECT_VALUE(object) ((Value){VALUE_OBJECT, {.obj = (Object*)object}})

/**
 * @brief Stores a series of values.
 */
typedef struct
{
    int capacity;
    int count;
    Value* values;
} ValueArray;

/**
 * @brief Initializes a ValueArray.
 * 
 * @param array A ValueArray to initialize.
 */
void InitValueArray(ValueArray* array);

/**
 * @brief Writes a single Value to a ValueArray.
 * 
 * @param array A ValueArray to write to.
 * @param value A Value to write.
 */
void WriteValueArray(ValueArray* array, Value value);

/**
 * @brief Frees a ValueArray.
 * 
 * @param array A ValueArray to free.
 */
void FreeValueArray(ValueArray* array);

/**
 * @brief Prints a Value in a human-readable form.
 * 
 * @param value A Value to print.
 */
void PrintValue(Value value);

/**
 * @brief Determines if two Values are equal.
 * 
 * @param a The first Value.
 * @param b The second Value.
 * @return true The two Values are equal.
 * @return false The two Values are not equal.
 */
bool AreValuesEqual(Value a, Value b);

#endif
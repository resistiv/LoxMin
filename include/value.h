#ifndef loxmin_value_h
#define loxmin_value_h

#include <string.h>
#include "common.h"

typedef struct Object Object;
typedef struct ObjectString ObjectString;

#ifdef NAN_BOXING

#define SIGN_BIT    ((uint64_t)0x8000000000000000)
#define QNAN        ((uint64_t)0x7ffc000000000000)
#define TAG_NIL     1
#define TAG_FALSE   2
#define TAG_TRUE    3

typedef uint64_t Value;

#define IS_BOOL(value)      (((value) | 1) == TRUE_VALUE)
#define IS_NIL(value)       ((value) == NIL_VALUE)
#define IS_NUMBER(value)    (((value) & QNAN) != QNAN)
#define IS_OBJECT(value)    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value)      ((value) == TRUE_VALUE)
#define AS_NUMBER(value)    ValueToNum(value)
#define AS_OBJECT(value)    ((Object*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define BOOL_VALUE(b)       ((b) ? TRUE_VALUE : FALSE_VALUE)
#define FALSE_VALUE         ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VALUE          ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VALUE           ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VALUE(num)   NumToValue(num)
#define OBJECT_VALUE(obj)   (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double ValueToNum(Value value)
{
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value NumToValue(double num)
{
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else

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

#endif

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

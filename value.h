#ifndef loxmin_value_h
#define loxmin_value_h

#include "common.h"

/**
 * @brief Represents a value.
 */
typedef double Value;

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

#endif
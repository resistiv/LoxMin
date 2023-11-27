#include <stdio.h>
#include "memory.h"
#include "value.h"

void InitValueArray(ValueArray* array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void WriteValueArray(ValueArray* array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int oldCap = array->capacity;
        array->capacity = GROW_CAPACITY(oldCap);
        array->values = GROW_ARRAY(Value, array->values, oldCap, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void FreeValueArray(ValueArray* array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    InitValueArray(array);
}

void PrintValue(Value value)
{
    printf("%g", AS_NUMBER(value));
}
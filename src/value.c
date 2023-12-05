#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
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
#ifdef NAN_BOXING
    if (IS_BOOL(value))
    {
        printf(AS_BOOL(value) ? "true" : "false");
    }
    else if (IS_NIL(value))
    {
        printf("nil");
    }
    else if (IS_NUMBER(value))
    {
        printf("%g", AS_NUMBER(value));
    }
    else if (IS_OBJECT(value))
    {
        PrintObject(value);
    }
#else
    switch (value.type)
    {
        case VALUE_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VALUE_NIL:
            printf("nil");
            break;
        case VALUE_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VALUE_OBJECT:
            PrintObject(value);
            break;
    }
#endif
}

bool AreValuesEqual(Value a, Value b)
{
#ifdef NAN_BOXING
    if (IS_NUMBER(a) && IS_NUMBER(b))
    {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }

    return a == b;
#else
    if (a.type != b.type)
    {
        return false;
    }
    else
    {
        switch (a.type)
        {
            case VALUE_BOOL:
                return AS_BOOL(a) == AS_BOOL(b);
            case VALUE_NIL:
                return true;
            case VALUE_NUMBER:
                return AS_NUMBER(a) == AS_NUMBER(b);
            case VALUE_OBJECT:
                return AS_OBJECT(a) == AS_OBJECT(b);
            // Unknown value
            default:
                return false;
        }
    }
#endif
}

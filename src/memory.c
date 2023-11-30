#include <stdlib.h>
#include "memory.h"
#include "vm.h"

static void FreeObject(Object* object);

void* Reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }
    else
    {
        void* result = realloc(pointer, newSize);

        // Make sure the operation succeeded.
        if (result == NULL)
        {
            exit(EXIT_FAILURE);
        }

        return result;
    }
}

void FreeObjects()
{
    Object* object = vm.objects;
    while (object != NULL)
    {
        Object* next = object->next;
        FreeObject(object);
        object = next;
    }
}

/**
 * @brief Frees an Object.
 * 
 * @param object An Object to free.
 */
static void FreeObject(Object* object)
{
    switch (object->type)
    {
        case OBJECT_FUNCTION:
        {
            ObjectFunction* function = (ObjectFunction*)object;
            FreeChunk(&function->chunk);
            FREE(ObjectFunction, object);
            break;
        }
        case OBJECT_NATIVE:
            FREE(ObjectNative, object);
            break;
        case OBJECT_STRING:
        {
            ObjectString* string = (ObjectString*)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjectString, object);
            break;
        }
    }
}

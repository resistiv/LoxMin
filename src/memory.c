#include <stdlib.h>
#include "memory.h"

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
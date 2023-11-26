#ifndef loxmin_memory_h
#define loxmin_memory_h

#include "common.h"

/**
 * @brief Determines a new array capacity based on the current capacity.
 */
#define GROW_CAPACITY(capacity) \
        ((capacity) < 8 ? 8 : (capacity) * 2)

/**
 * @brief Grows an array from an old size to a new size.
 */
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
        (type*)Reallocate(pointer, sizeof(type) * (oldCount), \
                          sizeof(type) * (newCount))

/**
 * @brief Frees an array.
 */
#define FREE_ARRAY(type, pointer, oldCount) \
        Reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * @brief Reallocates a piece of memory from one size to another.
 * 
 * @param pointer The location of the memory.
 * @param oldSize The old size of the memory.
 * @param newSize The new, desired size of the memory.
 * @return void* A pointer to the new, reallocated memory location.
 */
void* Reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif
#include <stdlib.h>
#include "chunk.h"
#include "memory.h"

void InitChunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
}

void FreeChunk(Chunk* chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    InitChunk(chunk);
}

void WriteChunk(Chunk* chunk, uint8_t byte)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCap = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCap);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCap, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;
}
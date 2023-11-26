#ifndef loxmin_chunk_h
#define loxmin_chunk_h

#include "common.h"

/**
 * @brief Enumerates all available opcodes.
 */
typedef enum
{
    OP_RETURN,
} OpCode;

/**
 * @brief Stores a series of instructions.
 */
typedef struct
{
    int count;
    int capacity;
    uint8_t* code;
} Chunk;

/**
 * @brief Initializes a Chunk.
 * 
 * @param chunk A Chunk to initialize.
 */
void InitChunk(Chunk* chunk);

/**
 * @brief Frees a Chunk.
 * 
 * @param chunk A Chunk to free.
 */
void FreeChunk(Chunk* chunk);

/**
 * @brief Writes a single byte to a Chunk.
 * 
 * @param chunk A Chunk to write to.
 * @param byte A byte to be written.
 */
void WriteChunk(Chunk* chunk, uint8_t byte);

#endif
#ifndef loxmin_chunk_h
#define loxmin_chunk_h

#include "common.h"
#include "value.h"

/**
 * @brief Enumerates all available opcodes.
 */
typedef enum
{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
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
    int* lines;
    ValueArray constants;
} Chunk;

/**
 * @brief Initializes a Chunk.
 * 
 * @param chunk A Chunk to initialize.
 */
void InitChunk(Chunk* chunk);

/**
 * @brief Writes a single byte to a Chunk.
 * 
 * @param chunk A Chunk to write to.
 * @param byte A byte to be written.
 * @param line The line of source code where the byte originates from.
 */
void WriteChunk(Chunk* chunk, uint8_t byte, int line);

/**
 * @brief Frees a Chunk.
 * 
 * @param chunk A Chunk to free.
 */
void FreeChunk(Chunk* chunk);

/**
 * @brief Adds a constant to a Chunk's constants ValueArray.
 * 
 * @param chunk A Chunk to add a constant to.
 * @param value The Value of the constant.
 * @return int The index where the constant was appended within the constants ValueArray.
 */
int AddConstant(Chunk* chunk, Value value);

#endif

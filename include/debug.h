#ifndef loxmin_debug_h
#define loxmin_debug_h

#include "chunk.h"

/**
 * @brief Disassembles all of the instructions of a Chunk.
 * 
 * @param chunk A Chunk to disassemble.
 * @param name The name of the Chunk.
 */
void DisassembleChunk(Chunk* chunk, const char* name);

/**
 * @brief Disassembles a single instruction.
 * 
 * @param chunk A Chunk holding the instruction.
 * @param offset The offset of the instruction.
 * @return int The offset of the next instruction.
 */
int DisassembleInstruction(Chunk* chunk, int offset);

#endif
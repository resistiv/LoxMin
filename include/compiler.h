#ifndef loxmin_compiler_h
#define loxmin_compiler_h

#include "object.h"
#include "vm.h"

/**
 * @brief Compiles a piece of source code.
 * 
 * @param source A source code string.
 */

/**
 * @brief Compiles a piece of source code to a Chunk.
 * 
 * @param source A source code string to compile.
 * @param chunk A resultant Chunk.
 * @return true If the compilation was successful.
 * @return false If the compilation failed.
 */
bool Compile(const char* source, Chunk* chunk);

#endif

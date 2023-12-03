#ifndef loxmin_compiler_h
#define loxmin_compiler_h

#include "object.h"
#include "vm.h"

/**
 * @brief Compiles a piece of source code to an ObjectFunction.
 * 
 * @param source A source code string to compile.
 * @return ObjectFunction* The resultant function.
 */
ObjectFunction* Compile(const char* source);

/**
 * @brief Marks all compiler roots for garbage collection.
 */
void MarkCompilerRoots();

#endif

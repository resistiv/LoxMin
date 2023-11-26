#ifndef loxmin_vm_h
#define loxmin_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

/**
 * @brief Stores the state of a virtual machine.
 */
typedef struct
{
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* sp;
} VM;

/**
 * @brief Enumerates all available interpretation results.
 */
typedef enum
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

/**
 * @brief Initializes the virtual machine.
 */
void InitVM();

/**
 * @brief Frees the virtual machine.
 */
void FreeVM();

/**
 * @brief Interprets a Chunk.
 * 
 * @param chunk A Chunk to interpret.
 * @return InterpretResult The result of the interpretation.
 */
InterpretResult Interpret(Chunk* chunk);

/**
 * @brief Pushes a Value onto the stack.
 * 
 * @param value A Value to push onto the stack.
 */
void Push(Value value);

/**
 * @brief Pops a Value off of the stack.
 * 
 * @return Value The popped Value.
 */
Value Pop();

#endif
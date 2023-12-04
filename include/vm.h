#ifndef loxmin_vm_h
#define loxmin_vm_h

#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/**
 * @brief Represents a call frame for a function call.
 */
typedef struct
{
    ObjectClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;


/**
 * @brief Stores the state of a virtual machine.
 */
typedef struct
{
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value* sp;
    Table globals;
    Table strings;
    ObjectString* initString;
    ObjectUpvalue* openUpvalues;

    size_t bytesAllocated;
    size_t nextGC;
    Object* objects;
    int grayCount;
    int grayCapacity;
    Object** grayStack;
} VM;

extern VM vm;

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
 * @brief Interprets a piece of source code.
 * 
 * @param source A source code string.
 * @return InterpretResult The result of the interpretation.
 */
InterpretResult Interpret(const char* source);

/**
 * @brief Pushes a Value onto the stack.
 * 
 * @param value A Value to push onto the stack.
 */
void StackPush(Value value);

/**
 * @brief Pops a Value off of the stack.
 * 
 * @return Value The popped Value.
 */
Value StackPop();

#endif

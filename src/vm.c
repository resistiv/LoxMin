#include <stdio.h>
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

static InterpretResult Run();
static Value Peek(int distance);
static void RuntimeError(const char* format, ...);

VM vm;

/**
 * @brief Resets the stack to an empty state.
 */
static void ResetStack()
{
    vm.sp = vm.stack;
}

void InitVM()
{
    ResetStack();
}

void FreeVM()
{

}

void Push(Value value)
{
    *(vm.sp++) = value;
}

Value Pop()
{
    return *(--vm.sp);
}

/**
 * @brief Peeks at an item on the stack at a specified depth.
 * 
 * @param distance The depth of the desired stack item.
 * @return Value The Value of stack item.
 */
static Value Peek(int distance)
{
    return vm.sp[-1 - distance];
}

InterpretResult Interpret(const char* source)
{
    Chunk chunk;
    InitChunk(&chunk);

    // Quit on compiler error
    if (!Compile(source, &chunk))
    {
        FreeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = Run();

    FreeChunk(&chunk);
    return result;
}

/**
 * @brief Runs the virtual machine on a piece of code.
 * 
 * @return InterpretResult The result of the code.
 */
static InterpretResult Run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
        do                      \
        {                       \
            double b = Pop();   \
            double a = Pop();   \
            Push(a op b);       \
        } while (false)

    while (1)
    {
        // Debug visibility routines
#ifdef DEBUG_TRACE_EXECUTION
        // Print stack contents
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.sp; slot++)
        {
            printf("[ ");
            PrintValue(*slot);
            printf(" ]");
        }
        printf("\n");

        // Disassemble and print current instruction
        DisassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction = READ_BYTE();
        switch (instruction)
        {
            case OP_CONSTANT:
                Value constant = READ_CONSTANT();
                Push(constant);
                break;
            case OP_ADD:
                BINARY_OP(+);
                break;
            case OP_SUBTRACT:
                BINARY_OP(-);
                break;
            case OP_MULTIPLY:
                BINARY_OP(*);
                break;
            case OP_DIVIDE:
                BINARY_OP(/);
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(Peek(0)))
                {
                    RuntimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                Push(NUMBER_VALUE(-AS_NUMBER(Pop())));
                break;
            case OP_RETURN:
                PrintValue(Pop());
                printf("\n");
                return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

/**
 * @brief Reports a runtime error and resets the stack.
 * 
 * @param format An error message with formatting.
 * @param ... 
 */
static void RuntimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    ResetStack();
}

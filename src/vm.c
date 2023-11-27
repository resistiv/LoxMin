#include <stdio.h>
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

static InterpretResult Run();

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

InterpretResult Interpret(const char* source)
{
    Compile(source);
    return INTERPRET_OK;
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
                Push(-Pop());
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

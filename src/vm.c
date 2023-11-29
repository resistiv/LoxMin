#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

static InterpretResult Run();
static Value StackPeek(int distance);
static bool IsFalsey(Value value);
static void ConcatenateStrings();
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
    vm.objects = NULL;
    InitTable(&vm.strings);
}

void FreeVM()
{
    FreeTable(&vm.strings);
    FreeObjects();
}

void StackPush(Value value)
{
    *(vm.sp++) = value;
}

Value StackPop()
{
    return *(--vm.sp);
}

/**
 * @brief Peeks at an item on the stack at a specified depth.
 * 
 * @param distance The depth of the desired stack item.
 * @return Value The Value of stack item.
 */
static Value StackPeek(int distance)
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
#define BINARY_OP(valueType, op) \
        do \
        { \
            if (!IS_NUMBER(StackPeek(0)) || !IS_NUMBER(StackPeek(1))) \
            { \
                RuntimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(StackPop()); \
            double a = AS_NUMBER(StackPop()); \
            StackPush(valueType(a op b)); \
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
                StackPush(constant);
                break;
            case OP_NIL:
                StackPush(NIL_VALUE);
                break;
            case OP_TRUE:
                StackPush(BOOL_VALUE(true));
                break;
            case OP_FALSE:
                StackPush(BOOL_VALUE(false));
                break;
            case OP_EQUAL:
                Value b = StackPop();
                Value a = StackPop();
                StackPush(BOOL_VALUE(AreValuesEqual(a, b)));
                break;
            case OP_GREATER:
                BINARY_OP(BOOL_VALUE, >);
                break;
            case OP_LESS:
                BINARY_OP(BOOL_VALUE, <);
                break;
            case OP_ADD:
                if (IS_STRING(StackPeek(0)) && IS_STRING(StackPeek(1)))
                {
                    ConcatenateStrings();
                }
                else if (IS_NUMBER(StackPeek(0)) && IS_NUMBER(StackPeek(1)))
                {
                    double b = AS_NUMBER(StackPop());
                    double a = AS_NUMBER(StackPop());
                    StackPush(NUMBER_VALUE(a + b));
                }
                else
                {
                    RuntimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VALUE, -);
                break;
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VALUE, *);
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VALUE, /);
                break;
            case OP_NOT:
                StackPush(BOOL_VALUE(IsFalsey(StackPop())));
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(StackPeek(0)))
                {
                    RuntimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                StackPush(NUMBER_VALUE(-AS_NUMBER(StackPop())));
                break;
            case OP_RETURN:
                PrintValue(StackPop());
                printf("\n");
                return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

/**
 * @brief Determines if a Value is "falsey."
 * 
 * @param value A Value to check.
 * @return true The Value is falsey.
 * @return false The Value is not falsey.
 */
static bool IsFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

/**
 * @brief Concatenates two strings from the stack.
 */
static void ConcatenateStrings()
{
    ObjectString* b = AS_STRING(StackPop());
    ObjectString* a = AS_STRING(StackPop());

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjectString* result = TakeString(chars, length);
    StackPush(OBJECT_VALUE(result));
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

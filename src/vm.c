#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

static InterpretResult Run();
static Value StackPeek(int distance);
static bool CallValue(Value callee, int argCount);
static bool Call(ObjectClosure* closure, int argCount);
static void DefineNative(const char* name, NativeFn function);
static Value ClockNative(int argCount, Value* args);
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
    vm.frameCount = 0;
}

void InitVM()
{
    ResetStack();
    vm.objects = NULL;
    InitTable(&vm.globals);
    InitTable(&vm.strings);

    DefineNative("clock", ClockNative);
}

void FreeVM()
{
    FreeTable(&vm.globals);
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
    ObjectFunction* function = Compile(source);
    if (function == NULL)
    {
        return INTERPRET_COMPILE_ERROR;
    }

    StackPush(OBJECT_VALUE(function));
    ObjectClosure* closure = NewClosure(function);
    StackPop();
    StackPush(OBJECT_VALUE(closure));
    Call(closure, 0);

    return Run();
}

/**
 * @brief Runs the virtual machine on a piece of code.
 * 
 * @return InterpretResult The result of the code.
 */
static InterpretResult Run()
{
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
        (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
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
        DisassembleInstruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));
#endif

        uint8_t instruction = READ_BYTE();
        switch (instruction)
        {
            case OP_CONSTANT:
            {
                Value constant = READ_CONSTANT();
                StackPush(constant);
                break;
            }
            case OP_NIL:
            {
                StackPush(NIL_VALUE);
                break;
            }
            case OP_TRUE:
            {
                StackPush(BOOL_VALUE(true));
                break;
            }
            case OP_FALSE:
            {
                StackPush(BOOL_VALUE(false));
                break;
            }
            case OP_POP:
            {
                StackPop();
                break;
            }
            case OP_GET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                StackPush(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL:
            {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = StackPeek(0);
                break;
            }
            case OP_GET_GLOBAL:
            {
                ObjectString* name = READ_STRING();
                Value value;
                if (!TableGet(&vm.globals, name, &value))
                {
                    RuntimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                StackPush(value);
                break;
            }
            case OP_DEFINE_GLOBAL:
            {
                ObjectString* name = READ_STRING();
                TableSet(&vm.globals, name, StackPeek(0));
                StackPop();
                break;
            }
            case OP_SET_GLOBAL:
            {
                ObjectString* name = READ_STRING();
                if (TableSet(&vm.globals, name, StackPeek(0)))
                {
                    TableDelete(&vm.globals, name);
                    RuntimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL:
            {
                Value b = StackPop();
                Value a = StackPop();
                StackPush(BOOL_VALUE(AreValuesEqual(a, b)));
                break;
            }
            case OP_GREATER:
            {
                BINARY_OP(BOOL_VALUE, >);
                break;
            }
            case OP_LESS:
            {
                BINARY_OP(BOOL_VALUE, <);
                break;
            }
            case OP_ADD:
            {
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
            }
            case OP_SUBTRACT:
            {
                BINARY_OP(NUMBER_VALUE, -);
                break;
            }
            case OP_MULTIPLY:
            {
                BINARY_OP(NUMBER_VALUE, *);
                break;
            }
            case OP_DIVIDE:
            {
                BINARY_OP(NUMBER_VALUE, /);
                break;
            }
            case OP_NOT:
            {
                StackPush(BOOL_VALUE(IsFalsey(StackPop())));
                break;
            }
            case OP_NEGATE:
            {
                if (!IS_NUMBER(StackPeek(0)))
                {
                    RuntimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                StackPush(NUMBER_VALUE(-AS_NUMBER(StackPop())));
                break;
            }
            case OP_PRINT:
            {
                PrintValue(StackPop());
                printf("\n");
                break;
            }
            case OP_JUMP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE:
            {
                uint16_t offset = READ_SHORT();
                if (IsFalsey(StackPeek(0)))
                {
                    frame->ip += offset;
                }
                break;
            }
            case OP_LOOP:
            {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL:
            {
                int argCount = READ_BYTE();
                if (!CallValue(StackPeek(argCount), argCount))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_CLOSURE:
            {
                ObjectFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjectClosure* closure = NewClosure(function);
                StackPush(OBJECT_VALUE(closure));
                break;
            }
            case OP_RETURN:
            {
                Value result = StackPop();
                vm.frameCount--;
                if (vm.frameCount == 0)
                {
                    StackPop();
                    return INTERPRET_OK;
                }
                
                vm.sp = frame->slots;
                StackPush(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

/**
 * @brief Calls an Object.
 * 
 * @param callee A Value being called.
 * @param argCount The number of arguments.
 * @return true If the call was successful.
 * @return false If the call failed.
 */
static bool CallValue(Value callee, int argCount)
{
    if (IS_OBJECT(callee))
    {
        switch (OBJECT_TYPE(callee))
        {
            case OBJECT_CLOSURE:
                return Call(AS_CLOSURE(callee), argCount);
            case OBJECT_NATIVE:
            {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.sp - argCount);
                vm.sp -= argCount + 1;
                StackPush(result);
                return true;
            }
            default:
                break;
        }
    }
    RuntimeError("Can only call functions and classes.");
    return false;
}

/**
 * @brief Calls a function.
 * 
 * @param closure A function closure to call.
 * @param argCount The number of arguments.
 * @return true The call succeeded.
 * @return false The call failed.
 */
static bool Call(ObjectClosure* closure, int argCount)
{
    if (argCount != closure->function->arity)
    {
        RuntimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX)
    {
        RuntimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.sp - argCount - 1;
    return true;
}

/**
 * @brief Defines a native function.
 * 
 * @param name The name of the function.
 * @param function The function.
 */
static void DefineNative(const char* name, NativeFn function)
{
    StackPush(OBJECT_VALUE(CopyString(name, (int)strlen(name))));
    StackPush(OBJECT_VALUE(NewNative(function)));
    TableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    StackPop();
    StackPop();
}

/**
 * @brief Handles timing functionality.
 * 
 * @param argCount Number of arguments.
 * @param args Arguments.
 * @return Value Return value.
 */
static Value ClockNative(int argCount, Value* args)
{
    return NUMBER_VALUE((double)clock() / CLOCKS_PER_SEC);
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

    for (int i = vm.frameCount - 1; i >= 0; i--)
    {
        CallFrame* frame = &vm.frames[i];
        ObjectFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if (function->name == NULL)
        {
            fprintf(stderr, "script\n");
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    ResetStack();
}

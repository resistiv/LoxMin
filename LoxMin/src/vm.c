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
static ObjectUpvalue* CaptureUpvalue(Value* local);
static void CloseUpvalues(Value* last);
static bool CallValue(Value callee, int argCount);
static bool Call(ObjectClosure* closure, int argCount);
static void DefineMethod(ObjectString* name);
static bool BindMethod(ObjectClass* _class, ObjectString* name);
static bool Invoke(ObjectString* name, int argCount);
static bool InvokeFromClass(ObjectClass* _class, ObjectString* name, int argCount);
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
    vm.openUpvalues = NULL;
}

void InitVM()
{
    ResetStack();
    vm.objects = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;

    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;

    InitTable(&vm.globals);
    InitTable(&vm.strings);

    vm.initString = NULL;
    vm.initString = CopyString("init", 4);

    DefineNative("clock", ClockNative);
}

void FreeVM()
{
    FreeTable(&vm.globals);
    FreeTable(&vm.strings);
    vm.initString = NULL;
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
            case OP_GET_UPVALUE:
            {
                uint8_t slot = READ_BYTE();
                StackPush(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE:
            {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = StackPeek(0);
                break;
            }
            case OP_GET_PROPERTY:
            {
                if (!IS_INSTANCE(StackPeek(0)))
                {
                    RuntimeError("Only instances have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjectInstance* instance = AS_INSTANCE(StackPeek(0));
                ObjectString* name = READ_STRING();

                // Search for a field first
                Value value;
                if (TableGet(&instance->fields, name, &value))
                {
                    StackPop();
                    StackPush(value);
                    break;
                }

                // No field, assume method and search
                if (!BindMethod(instance->_class, name))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_PROPERTY:
            {
                if (!IS_INSTANCE(StackPeek(1)))
                {
                    RuntimeError("Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjectInstance* instance = AS_INSTANCE(StackPeek(1));
                TableSet(&instance->fields, READ_STRING(), StackPeek(0));
                Value value = StackPop();
                StackPop();
                StackPush(value);
                break;
            }
            case OP_GET_SUPER:
            {
                ObjectString* name = READ_STRING();
                ObjectClass* superclass = AS_CLASS(StackPop());

                if (!BindMethod(superclass, name))
                {
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
                fflush(stdout);
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
            case OP_INVOKE:
            {
                ObjectString* method = READ_STRING();
                int argCount = READ_BYTE();
                if (!Invoke(method, argCount))
                {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_SUPER_INVOKE:
            {
                ObjectString* method = READ_STRING();
                int argCount= READ_BYTE();
                ObjectClass* superclass = AS_CLASS(StackPop());
                if (!InvokeFromClass(superclass, method, argCount))
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
                for (int i = 0; i < closure->upvalueCount; i++)
                {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal)
                    {
                        closure->upvalues[i] = CaptureUpvalue(frame->slots + index);
                    }
                    else
                    {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:
            {
                CloseUpvalues(vm.sp - 1);
                StackPop();
                break;
            }
            case OP_RETURN:
            {
                Value result = StackPop();
                CloseUpvalues(frame->slots);
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
            case OP_CLASS:
            {
                StackPush(OBJECT_VALUE(NewClass(READ_STRING())));
                break;
            }
            case OP_INHERIT:
            {
                Value superclass = StackPeek(1);
                if (!IS_CLASS(superclass))
                {
                    RuntimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjectClass* subclass = AS_CLASS(StackPeek(0));
                TableCopy(&AS_CLASS(superclass)->methods, &subclass->methods);
                StackPop();
                break;
            }
            case OP_METHOD:
            {
                DefineMethod(READ_STRING());
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
 * @brief Captures a local Value as an upvalue.
 * 
 * @param local A local to capture.
 * @return ObjectUpvalue* A resulting upvalue object.
 */
static ObjectUpvalue* CaptureUpvalue(Value* local)
{
    // Look for existing upvalue, don't duplicate!
    ObjectUpvalue* prevUpvalue = NULL;
    ObjectUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local)
    {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local)
    {
        return upvalue;
    }

    ObjectUpvalue* newUpvalue = NewUpvalue(local);
    newUpvalue->next = upvalue;

    if (prevUpvalue == NULL)
    {
        vm.openUpvalues = newUpvalue;
    }
    else
    {
        prevUpvalue->next = newUpvalue;
    }

    return newUpvalue;
}

/**
 * @brief Closes upvalues and moves them to the heap.
 * 
 * @param last The last upvalue slot to close.
 */
static void CloseUpvalues(Value* last)
{
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last)
    {
        ObjectUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
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
            case OBJECT_BOUND_METHOD:
            {
                ObjectBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm.sp[-argCount - 1] = bound->receiver;
                return Call(bound->method, argCount);
            }
            case OBJECT_CLASS:
            {
                ObjectClass* _class = AS_CLASS(callee);
                vm.sp[-argCount - 1] = OBJECT_VALUE(NewInstance(_class));
                Value initializer;
                if (TableGet(&_class->methods, vm.initString, &initializer))
                {
                    return Call(AS_CLOSURE(initializer), argCount);
                }
                else if (argCount != 0)
                {
                    RuntimeError("Expected 0 arguments but got %d.", argCount);
                    return false;
                }
                return true;
            }
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
 * @brief Defines a class method.
 * 
 * @param name The name of the method.
 */
static void DefineMethod(ObjectString* name)
{
    Value method = StackPeek(0);
    ObjectClass* _class = AS_CLASS(StackPeek(1));
    TableSet(&_class->methods, name, method);
    StackPop();
}

/**
 * @brief Binds a method to a class.
 * 
 * @param _class A class to bind the method to.
 * @param name The name of the method.
 * @return true The binding was successful.
 * @return false The binding failed.
 */
static bool BindMethod(ObjectClass* _class, ObjectString* name)
{
    // Verify the method we're looking for exists within the class
    Value method;
    if (!TableGet(&_class->methods, name, &method))
    {
        RuntimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjectBoundMethod* bound = NewBoundMethod(StackPeek(0), AS_CLOSURE(method));

    StackPop();
    StackPush(OBJECT_VALUE(bound));
    return true;
}

/**
 * @brief Directly invokes a method.
 * 
 * @param name The name of the method.
 * @param argCount The number of arguments fed to the method.
 * @return true If the invocation succeeded.
 * @return false If the invocation failed.
 */
static bool Invoke(ObjectString* name, int argCount)
{
    Value receiver = StackPeek(argCount);

    if (!IS_INSTANCE(receiver))
    {
        RuntimeError("Only instances have methods.");
        return false;
    }

    ObjectInstance* instance = AS_INSTANCE(receiver);

    Value value;
    if (TableGet(&instance->fields, name, &value))
    {
        vm.sp[-argCount - 1] = value;
        return CallValue(value, argCount);
    }

    return InvokeFromClass(instance->_class, name, argCount);
}

/**
 * @brief Directly invokes a method from a class.
 * 
 * @param _class The class containing the method.
 * @param name The name of the method.
 * @param argCount The number of arguments fed to the method.
 * @return true If the invocation succeeded.
 * @return false If the invocation failed.
 */
static bool InvokeFromClass(ObjectClass* _class, ObjectString* name, int argCount)
{
    Value method;
    if (!TableGet(&_class->methods, name, &method))
    {
        RuntimeError("Undefined property '%s'.", name->chars);
        return false;
    }
    return Call(AS_CLOSURE(method), argCount);
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
    ObjectString* b = AS_STRING(StackPeek(0));
    ObjectString* a = AS_STRING(StackPeek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjectString* result = TakeString(chars, length);
    StackPop();
    StackPop();
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

#include <stdlib.h>
#include "compiler.h"
#include "memory.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

static void FreeObject(Object* object);
static void MarkRoots();
static void MarkArray(ValueArray* array);
static void TraceReferences();
static void BlackenObject(Object* object);
static void Sweep();

void* Reallocate(void* pointer, size_t oldSize, size_t newSize)
{
    vm.bytesAllocated += newSize - oldSize;
    if (newSize > oldSize)
    {
#ifdef DEBUG_STRESS_GC
        CollectGarbage();
#endif

        if (vm.bytesAllocated > vm.nextGC)
        {
            CollectGarbage();
        }
    }

    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }
    else
    {
        void* result = realloc(pointer, newSize);

        // Make sure the operation succeeded.
        if (result == NULL)
        {
            exit(EXIT_FAILURE);
        }

        return result;
    }
}

void CollectGarbage()
{
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = vm.bytesAllocated;
#endif

    MarkRoots();
    TraceReferences();
    TableRemoveWhite(&vm.strings);
    Sweep();

    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n", before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}

/**
 * @brief Marks the roots of all objects.
 */
static void MarkRoots()
{
    for (Value* slot = vm.stack; slot < vm.sp; slot++)
    {
        MarkValue(*slot);
    }

    for (int i = 0; i < vm.frameCount; i++)
    {
        MarkObject((Object*)vm.frames[i].closure);
    }

    for (ObjectUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next)
    {
        MarkObject((Object*)upvalue);
    }

    MarkTable(&vm.globals);
    MarkCompilerRoots();
    MarkObject((Object*)vm.initString);
}

/**
 * @brief Marks an array as accessible.
 * 
 * @param array A ValueArray to mark.
 */
static void MarkArray(ValueArray* array)
{
    for (int i = 0; i < array->count; i++)
    {
        MarkValue(array->values[i]);
    }
}

void MarkValue(Value value)
{
    if (IS_OBJECT(value))
    {
        MarkObject(AS_OBJECT(value));
    }
}

void MarkObject(Object* object)
{
    if (object == NULL)
    {
        return;
    }
    if (object->isMarked)
    {
        return;
    }

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    PrintValue(OBJECT_VALUE(object));
    printf("\n");
#endif

    object->isMarked = true;

    if (vm.grayCapacity < vm.grayCount + 1)
    {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = (Object**)realloc(vm.grayStack, sizeof(Object*) * vm.grayCapacity);

        // Fail to allocate gray item stack, die
        if (vm.grayStack == NULL)
        {
            exit(EXIT_FAILURE);
        }
    }

    vm.grayStack[vm.grayCount++] = object;
}

void FreeObjects()
{
    Object* object = vm.objects;
    while (object != NULL)
    {
        Object* next = object->next;
        FreeObject(object);
        object = next;
    }

    free(vm.grayStack);
}

/**
 * @brief Frees an Object.
 * 
 * @param object An Object to free.
 */
static void FreeObject(Object* object)
{
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void*)object, object->type);
#endif

    switch (object->type)
    {
        case OBJECT_BOUND_METHOD:
        {
            FREE(ObjectBoundMethod, object);
            break;
        }
        case OBJECT_CLASS:
        {
            ObjectClass* _class = (ObjectClass*)object;
            FreeTable(&_class->methods);
            FREE(ObjectClass, object);
            break;
        }
        case OBJECT_INSTANCE:
        {
            ObjectInstance* instance = (ObjectInstance*)object;
            FreeTable(&instance->fields);
            FREE(ObjectInstance, object);
            break;
        }
        case OBJECT_UPVALUE:
        {
            FREE(ObjectUpvalue, object);
            break;
        }
        case OBJECT_CLOSURE:
        {
            ObjectClosure* closure = (ObjectClosure*)object;
            FREE_ARRAY(ObjectUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(ObjectClosure, object);
            break;
        }
        case OBJECT_FUNCTION:
        {
            ObjectFunction* function = (ObjectFunction*)object;
            FreeChunk(&function->chunk);
            FREE(ObjectFunction, object);
            break;
        }
        case OBJECT_NATIVE:
            FREE(ObjectNative, object);
            break;
        case OBJECT_STRING:
        {
            ObjectString* string = (ObjectString*)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjectString, object);
            break;
        }
    }
}

/**
 * @brief Traces all references of gray items.
 */
static void TraceReferences()
{
    while (vm.grayCount > 0)
    {
        Object* object = vm.grayStack[--vm.grayCount];
        BlackenObject(object);
    }
}

/**
 * @brief Blackens an object by traversing its references.
 * 
 * @param object An Object to blacken.
 */
static void BlackenObject(Object* object)
{
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    PrintValue(OBJECT_VALUE(object));
    printf("\n");
#endif

    switch (object->type)
    {
        case OBJECT_BOUND_METHOD:
        {
            ObjectBoundMethod* bound = (ObjectBoundMethod*)object;
            MarkValue(bound->receiver);
            MarkObject((Object*)bound->method);
            break;
        }
        case OBJECT_CLASS:
        {
            ObjectClass* _class = (ObjectClass*)object;
            MarkObject((Object*)_class->name);
            MarkTable(&_class->methods);
            break;
        }
        case OBJECT_INSTANCE:
        {
            ObjectInstance* instance = (ObjectInstance*)object;
            MarkObject((Object*)instance->_class);
            MarkTable(&instance->fields);
            break;
        }
        case OBJECT_CLOSURE:
        {
            ObjectClosure* closure = (ObjectClosure*)object;
            MarkObject((Object*)closure->function);
            for (int i = 0; i < closure->upvalueCount; i++)
            {
                MarkObject((Object*)closure->upvalues[i]);
            }
            break;
        }
        case OBJECT_FUNCTION:
        {
            ObjectFunction* function = (ObjectFunction*)object;
            MarkObject((Object*)function->name);
            MarkArray(&function->chunk.constants);
            break;
        }
        case OBJECT_UPVALUE:
            MarkValue(((ObjectUpvalue*)object)->closed);
            break;
        case OBJECT_NATIVE:
        case OBJECT_STRING:
            break;
    }
}

/**
 * @brief Deletes all remaining white objects.
 */
static void Sweep()
{
    Object* previous = NULL;
    Object* object = vm.objects;
    while (object != NULL)
    {
        if (object->isMarked)
        {
            object->isMarked = false;
            previous = object;
            object = object->next;
        }
        else
        {
            Object* unreached = object;
            object = object->next;
            if (previous != NULL)
            {
                previous->next = object;
            }
            else
            {
                vm.objects = object;
            }

            FreeObject(unreached);
        }
    }
}

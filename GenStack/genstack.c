#include <stdlib.h>
#include "genstack.h"
#include "../GenVector/Vector.h"


struct Stack
{
    Vector* m_vector;
};

static StackResult VectorResultConverter(VectorResult _vectorResult);


Stack* StackCreate(size_t _initialCapacity, size_t _blockSize)
{
    if (_initialCapacity == 0 && _blockSize == 0)
    {
        return NULL;
    }

    Stack *stack = malloc(sizeof *stack);
    if (!stack) 
    { 
        return NULL; 
    }

    stack->m_vector = VectorCreate(_initialCapacity, _blockSize);
    if (!stack->m_vector)
    {
        free(stack);
        return NULL;
    }

    return stack;
}

void StackDestroy(Stack** _stack, void (*_elementDestroy)(void* _item))
{
    if (_stack == NULL || *_stack == NULL)
    {
        return;
    }

    Vector *vector = (*_stack)->m_vector;
    VectorDestroy(&vector, _elementDestroy);

    free(*_stack);
    *_stack = NULL;

    return;
}

StackResult StackPush(Stack* _stack, void* _item)
{
    if (_stack == NULL) { return STACK_UNINITIALIZED_ERROR; }
    if (_item == NULL)  { return STACK_NULL_PTR; }

    VectorResult pushResult = VectorAppend(_stack->m_vector, _item);

    return VectorResultConverter(pushResult);
}

StackResult StackPop(Stack* _stack, void** _pValue)
{
    if (_stack == NULL) { return STACK_UNINITIALIZED_ERROR; }
    if (_pValue == NULL) { return STACK_NULL_PTR; }

    VectorResult popResult = VectorRemove(_stack->m_vector, _pValue);
    return VectorResultConverter(popResult);
}

StackResult StackTop(Stack* _stack, void** _pValue)
{
    if (_stack == NULL) { return STACK_UNINITIALIZED_ERROR; }
    if (_pValue == NULL) { return STACK_NULL_PTR; }
    if (VectorSize(_stack->m_vector) == 0) { return STACK_SIZE_ZERO_ERROR; }

    size_t topIndex = VectorSize(_stack->m_vector) - 1;
    VectorResult topResult = VectorGet(_stack->m_vector, topIndex, _pValue);

    return VectorResultConverter(topResult);
}

size_t StackSize(const Stack* _stack)
{
    return (_stack == NULL) ? 0 : VectorSize(_stack->m_vector);
}

size_t StackCapacity(const Stack* _stack)
{
    return (_stack == NULL) ? 0 : VectorCapacity(_stack->m_vector);
}

int StackIsEmpty(Stack* _stack)
{
    return (_stack == NULL) ? 1 : (VectorSize(_stack->m_vector) == 0);
}

size_t StackPrint(const Stack* _stack, StackElementAction _action, void* _context)
{
    return (_stack == NULL || _action == NULL) ? 0 : VectorForEach(_stack->m_vector, _action, _context);
}

static StackResult VectorResultConverter(VectorResult _vectorResult)
{
    switch (_vectorResult)
    {
        case VECTOR_SUCCESS:
            return STACK_SUCCESS;

        case VECTOR_UNINITIALIZED_ERROR:
            return STACK_UNINITIALIZED_ERROR;

        case VECTOR_ALLOCATION_ERROR:
            return STACK_ALLOCATION_ERROR;

        case VECTOR_SIZE_ZERO_ERROR:
            return STACK_SIZE_ZERO_ERROR;

        case VECTOR_OVERFLOW_ERROR:
            return STACK_OVERFLOW_ERROR;

        case VECTOR_NULL_PTR:
            return STACK_NULL_PTR;

        // VECTOR_INDEX_OUT_OF_BOUNDS is unexpected from Stack and falls through to default
        default:
            return STACK_UNKNOWN_ERROR; /* fallback */
    }
}

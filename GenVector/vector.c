#include "vector.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#define SAFE_REALLOC(ptr, count) realloc((ptr), (count) * sizeof(*(ptr)))
#define VECTOR_GET(vector, index, value) (*(value) = (vector)->m_voidArray[index])
struct Vector
{
    void **m_voidArray;     // The array of pointers
    size_t m_capacity;  // Total number of elements allocated in memory 
    size_t m_size;      // Current number of elements actually stored
    size_t m_blockSize; /* the chunk size to be allocated when no space*/
    size_t m_initialCapacity;
};

// Private functions:
static VectorResult VectorExpand(Vector *vector);
static bool IsFull(const Vector *vector);
static VectorResult VectorNullCheck(Vector const *_vector);
static bool VectorShouldShrink(Vector const *_vector);

Vector* VectorCreate(size_t _initialCapacity, size_t _blockSize)
{
    if (_initialCapacity == 0 && _blockSize == 0)
    {
        return NULL;
    }

    Vector *vectorPtr = malloc(sizeof(Vector));
    if (vectorPtr == NULL)
    {
        perror("Error initializing memory for vector");
        return NULL;
    }

    vectorPtr->m_blockSize = _blockSize;
    vectorPtr->m_capacity = _initialCapacity;
    vectorPtr->m_initialCapacity = _initialCapacity;
    vectorPtr->m_size = 0;
    vectorPtr->m_voidArray = malloc(_initialCapacity * sizeof(void*));

    if (vectorPtr->m_voidArray == NULL)
    {
        perror("Couldn't allocate m_voidArray");
        free(vectorPtr);
        return NULL;
    }
    return vectorPtr;
}

void VectorDestroy(Vector **_vector, void (*_elementDestroy)(void *_item))
{
    if (_vector == NULL || *_vector == NULL) { return; }

    Vector *vectorPTR = *_vector;
    if (_elementDestroy != NULL)
    {
        for (size_t i = 0 ; i < vectorPTR->m_size ; ++i)
        {
            _elementDestroy(vectorPTR->m_voidArray[i]);
        }
    }

    free(vectorPTR->m_voidArray);
    free(vectorPTR);
    *_vector = NULL;
}

VectorResult VectorAppend(Vector *_vector, void *_item)
{

    VectorResult status = VectorNullCheck(_vector);
    if (status != VECTOR_SUCCESS) { return status; }

    if (IsFull(_vector))
    {
        status = VectorExpand(_vector);
        if (status != VECTOR_SUCCESS) { return status; }
    }

    _vector->m_voidArray[_vector->m_size++] = _item;

    return VECTOR_SUCCESS;
}

VectorResult VectorRemove(Vector* _vector, void** _pValue)
{
    // Check if vector is empty or null
    if (_vector == NULL)            { return VECTOR_NULL_PTR; }
    if (_pValue == NULL)            { return VECTOR_NULL_PTR; }
    if (_vector->m_size == 0)       { return VECTOR_SIZE_ZERO_ERROR; }

    // Pop_back
    *_pValue = _vector->m_voidArray[--_vector->m_size];
    
    if (VectorShouldShrink(_vector))
    {
        const size_t newCapacity = (_vector->m_capacity) - (_vector->m_blockSize);
        void** newVoidArray = SAFE_REALLOC(_vector->m_voidArray, newCapacity);
        if (newVoidArray)
        {
            _vector->m_voidArray = newVoidArray;
            _vector->m_capacity = newCapacity;
        }
    }

    return VECTOR_SUCCESS;  
}


VectorResult VectorGet(Vector const *_vector, size_t _index, void **_pValue)
{
    VectorResult status = VectorNullCheck(_vector);
    if (status != VECTOR_SUCCESS)   { return status; }
    if (_pValue == NULL)            { return VECTOR_NULL_PTR; }
    if (_index >= _vector->m_size)  { return VECTOR_INDEX_OUT_OF_BOUNDS_ERROR; }

    *_pValue = _vector->m_voidArray[_index];

    return VECTOR_SUCCESS;
}

VectorResult VectorSet(Vector *_vector, size_t _index, void *_value)
{
    if (_vector == NULL)           { return VECTOR_UNINITIALIZED_ERROR; }
    if (_index >= _vector->m_size) { return VECTOR_INDEX_OUT_OF_BOUNDS_ERROR; }

    _vector->m_voidArray[_index] = _value;

    return VECTOR_SUCCESS;;
}

size_t VectorSize(const Vector* _vector)
{
    return (_vector == NULL) ? 0 : _vector->m_size;
}

size_t VectorCapacity(Vector const *_vector)
{
    return (_vector == NULL) ? 0 : _vector->m_capacity;
}

size_t VectorForEach(const Vector* _vector, VectorElementAction _action, void* _context)
{

    VectorResult status = VectorNullCheck(_vector);
    if (status != VECTOR_SUCCESS) { return 0; }
    
    size_t vectorSize = _vector->m_size;
    size_t i;

    for (i = 0 ; i < vectorSize ; ++i)
    {
        void *currElement;
        VECTOR_GET(_vector, i, &currElement);
        if (_action(currElement, i, _context) == 0)
        {
            break;
        }
    }
       

    return i;
}

/*///////////////////////////////////////////////////////////////
            Static Helpers
///////////////////////////////////////////////////////////////*/

static bool VectorShouldShrink(Vector const *_vector)
{
    return (_vector->m_capacity > _vector->m_initialCapacity) 
        ? _vector->m_size < _vector->m_capacity - 2 * _vector->m_blockSize 
        : false;
}


static bool IsFull(const Vector *vector)
{
    return vector->m_size == vector->m_capacity;
}


static VectorResult VectorExpand(Vector *_vector)
{
    if (_vector->m_blockSize == 0)
    {
        return VECTOR_OVERFLOW_ERROR;
    }

    size_t newSize = _vector->m_capacity + _vector->m_blockSize;
    void** resizedVoidArray = SAFE_REALLOC(_vector->m_voidArray, newSize);
    
    if (resizedVoidArray)
    {
        _vector->m_voidArray = resizedVoidArray;
        _vector->m_capacity = newSize;
        return VECTOR_SUCCESS;
    }

    return VECTOR_ALLOCATION_ERROR;
}

static VectorResult VectorNullCheck(Vector const *_vector)
{
    return (_vector == NULL) ? VECTOR_UNINITIALIZED_ERROR : VECTOR_SUCCESS;
}

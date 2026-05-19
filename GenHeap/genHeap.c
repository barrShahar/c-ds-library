#include "genHeap.h"
#include <stdlib.h>
#include <stdbool.h>


#define HEAP_FOR(i,N) for (size_t (i) = 0 ; (i) < (N) ; ++(i))
#define HEAP_CHILD_LEFT(x)  (((x) * 2) + 1)
#define HEAP_CHILD_RIGHT(x) (((x) * 2) + 2)
#define HEAP_PARENT(x)      (((x) - 1) / 2)
#define HEAP_SIZE(heap) VectorSize(heap->m_vector)

static HeapResultCode HeapVectorResult2HeapResult(const VectorResult _vecResult);
static void HeapifyUp(Heap* _heap, size_t _index);
static void HeapifyDown(Heap* _heap, size_t _index, size_t _size);
static bool HeapSwap(Vector* _vector, size_t lIdx, size_t rIdx);
static HeapResultCode HeapCheckParamsEl(Heap* _heap, void* _element, bool _checkElement);
static bool HeapHeapifyDownStop(void* parent, void* leftChild, void* rightChild, bool _hasRightChild, Comparator comp);

struct Heap
{
    Vector* m_vector;
    Comparator m_comparator;
};

Heap* HeapBuild(Vector* _vector, Comparator _pfComp)
{
    if (!_vector || !_pfComp) { return NULL; }

    Heap* heap = malloc(sizeof(Heap));
    if (!heap) { return NULL; }

    heap->m_vector = _vector;
    heap->m_comparator = _pfComp;

    size_t size = VectorSize(_vector);
    for (size_t i = (size / 2) ; i < size ; ++i)
    {
        HeapifyDown(heap, size - 1 - i, size);
    }
    

    return heap;
}

Vector* HeapDestroy(Heap** _heap)
{
    if (!_heap || !*_heap) { return NULL; }

    Vector* retVal = (*_heap)->m_vector;
    free(*_heap);
    *_heap = NULL;

    return retVal;
}

const void* HeapPeek(const Heap* _heap)
{
    if (!_heap) { return NULL; }

    void* topItem;
    if (VectorGet(_heap->m_vector, 0, &topItem) != VECTOR_SUCCESS)
    {
        return NULL;
    }

    return topItem;
}

HeapResultCode HeapInsert(Heap* _heap, void* _element)
{
    HeapResultCode status = HeapCheckParamsEl(_heap, _element, true);
    if (status != HEAP_SUCCESS) { return status; }

    VectorResult appendStatus = VectorAppend(_heap->m_vector, _element);
    if (appendStatus != VECTOR_SUCCESS)
    {
        return HeapVectorResult2HeapResult(appendStatus);
    }

    HeapifyUp(_heap, HEAP_SIZE(_heap) - 1);

    return HEAP_SUCCESS;
}


void* HeapExtract(Heap* _heap)
{
    if (!_heap || !_heap->m_vector) { return NULL; }
    void* item;

    size_t size = VectorSize(_heap->m_vector);
    if (size == 0) 
    {
        return NULL;
    }
    HeapSwap(_heap->m_vector, 0, size - 1);

    if (VectorRemove(_heap->m_vector, &item) != VECTOR_SUCCESS)
    {
        return NULL;
    }

    HeapifyDown(_heap, 0, size - 1);

    return item;
}

size_t HeapSize(const Heap* _heap)
{
    return _heap ? VectorSize(_heap->m_vector) : 0;
}

size_t HeapForEach(const Heap* _heap, int(*_act)(const void *_elem, void * _context), void* _context)
{
    if (!_heap || !_act) { return 0; }
    void* item;
    size_t i;
    for (i = 0 ; i < HEAP_SIZE(_heap) ; ++i)
    {
        VectorGet(_heap->m_vector, i, &item);
        if (_act(item, _context) == 0)
        {
            break;
        }
    }

    return i;
}

/////////// statics //////////////////
static HeapResultCode HeapCheckParamsEl(Heap* _heap, void* _element, bool _checkElement)
{
    if (!_heap) { return HEAP_NOT_INITIALIZED; }
    if (!_heap->m_vector) { return HEAP_CORRUPTED_DATA; }
    if (_checkElement && !_element) { return HEAP_NULL_ITEM; }
    return HEAP_SUCCESS;
}

static HeapResultCode HeapVectorResult2HeapResult(const VectorResult _vecResult)
{
    switch (_vecResult) 
    {
    case VECTOR_SUCCESS:
        return HEAP_SUCCESS;
        
    case VECTOR_UNINITIALIZED_ERROR:
        return HEAP_CORRUPTED_DATA;
        
    case VECTOR_ALLOCATION_ERROR:
        return HEAP_REALLOCATION_FAILED;
    
    case VECTOR_OVERFLOW_ERROR:
        return HEAP_OVERFLOW_ERROR;
        
    case VECTOR_SIZE_ZERO_ERROR:
        return HEAP_EMPTY;
        
    default:
        return HEAP_UNKNOWN_ERROR;
    }

    return HEAP_UNKNOWN_ERROR;
}

static bool HeapSwap(Vector* _vector, size_t lIdx, size_t rIdx)
{
    void* tmp;
    void* right;
    VectorResult resTmp = VectorGet(_vector, lIdx, &tmp); // tmp = v[left]
    VectorResult resRight = VectorGet(_vector, rIdx, &right); // right = v[right]

    VectorResult setLeft = VectorSet(_vector, lIdx, right); // v[left] = right
    VectorResult setRight = VectorSet(_vector, rIdx, tmp); // v[right] = tmp

    VectorResult arr[] = {resTmp, resRight, setLeft, setRight};
    HEAP_FOR(i, sizeof(arr)/sizeof(arr[0]))
    {
        if (arr[i] != VECTOR_SUCCESS)
        {
            return false;
        }
    } 

    return true;;
}

static void HeapifyDown(Heap* _heap, size_t _index, size_t _size)
{
    if (_index > _size/2)
    {
        return; 
    }

    void* leftChild;
    void* rightChild;
    void* parent;

    Vector* vector = _heap->m_vector; // Alias
    Comparator comp = _heap->m_comparator; // Alias

    VectorGet(vector, _index, &parent);
    VectorGet(vector, HEAP_CHILD_LEFT(_index), &leftChild); 
    VectorResult resRight = VectorGet(vector, HEAP_CHILD_RIGHT(_index), &rightChild);

    if (HeapHeapifyDownStop(parent, leftChild, rightChild, resRight == VECTOR_SUCCESS, comp))
    {
        return;
    }

    if (resRight == VECTOR_INDEX_OUT_OF_BOUNDS_ERROR)
    {
        HeapSwap(vector, _index, HEAP_CHILD_LEFT(_index));
        HeapifyDown(_heap, HEAP_CHILD_LEFT(_index), _size);
        return;
    }
    int cmpChilds = comp(leftChild, rightChild);
    
    if (cmpChilds == 0)
    {
        HeapSwap(vector, _index, HEAP_CHILD_LEFT(_index));
        HeapifyDown(_heap, HEAP_CHILD_LEFT(_index), _size);
        return;
    }
    if (cmpChilds < 0)
    {
        HeapSwap(vector, _index, HEAP_CHILD_LEFT(_index));
        HeapifyDown(_heap, HEAP_CHILD_LEFT(_index), _size);
        return;
    }
    // else
    HeapSwap(vector, _index, HEAP_CHILD_RIGHT(_index));
    HeapifyDown(_heap, HEAP_CHILD_RIGHT(_index), _size);
}

static void HeapifyUp(Heap* _heap, size_t _index)
{
    Vector* vector = _heap->m_vector; // Alias
    Comparator comp = _heap->m_comparator; // Alias

    size_t currIdx = _index;
    void* currItem;
    void* parentItem;
    VectorGet(vector, _index, &currItem);

    while(currIdx > 0)
    {
        size_t parentIdx = HEAP_PARENT(currIdx);
        VectorGet(vector, parentIdx, &parentItem);

        int cmp = comp(currItem, parentItem);
        if (cmp >= 0) { return; }

        HeapSwap(vector, currIdx, parentIdx);
        currIdx = parentIdx;
    }

    return;
}

static bool HeapHeapifyDownStop(void* parent, void* leftChild, void* rightChild, bool _hasRightChild, Comparator comp)
{
    if (comp(parent, leftChild) > 0)
    {
        return false;
    }

    if (_hasRightChild && comp(parent, rightChild) > 0)
    {
        return false;
    }

    return true;
}
#include <stdio.h>
#include "../genHeap.h"
#include <stdlib.h>
#include <assert.h>

#define NEW_LINE printf("\n")
#define FOR(i,N) for (size_t (i) = 0 ; (i) < (N) ; ++(i))

static int CompareInts(const void* _left, const void* _right)
{
    return (*(int*)_right - *(int*)_left);
}

static int PrintInt(const void* integer, void* _context)
{
    printf("%d, ", *(int*)integer);
    (void)_context;
    return 1;
}

static void PrintHeapIntAction(Heap* _heap, void* _context)
{
    HeapForEach(_heap, PrintInt, NULL);
    NEW_LINE;
    (void)_context;
}

int PrintVectorAction(void* _element, size_t _index, void* _context)
{
    printf("%d, ", *(int*)_element);
    (void)_index;
    (void)_context;
    return 1;
}

static void TestHeapBuild()
{
    printf("**TestHeapBuild**\n");
    Vector* vector = VectorCreate(2, 2);
    FOR(i, 10)
    {
        int* number = malloc(sizeof(int));
        *number = i;
        assert(VectorAppend(vector, number) == VECTOR_SUCCESS);
    }
    printf("Print Vector:\n");
    VectorForEach(vector, PrintVectorAction, NULL);
    NEW_LINE;

    Heap* heap = HeapBuild(vector, CompareInts);
    printf("Print Heap:\n");
    PrintHeapIntAction(heap, NULL);


    vector = HeapDestroy(&heap);

}

static void DestroyInt(void* integer)
{
    free((int*)integer);
}

void TestHeapInsert()
{
    printf("**TestHeapInsert**\n");
    Vector* vector = VectorCreate(2, 2);
    Heap* heap = HeapBuild(vector, CompareInts);

    FOR(i, 10)
    {
        int* number = malloc(sizeof(int));
        *number = i;
        assert(HeapInsert(heap, number) == HEAP_SUCCESS);

        const int* top = HeapPeek(heap);
        assert(*top == *number);
    }
    
    PrintHeapIntAction(heap, NULL);
    HeapDestroy(&heap);
    VectorDestroy(&vector, DestroyInt);
}


int main(void)
{
    TestHeapBuild();
    TestHeapInsert();
    return 0;
}

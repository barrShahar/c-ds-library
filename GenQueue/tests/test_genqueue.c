#include "../genqueue.h"
#include "stdio.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define TYPE int
#define UNUSED(x) (void)(x)
#define LENGTH(array) (sizeof(array) / sizeof(array[0]))

/*
  For testing memory leaks run:
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_q
*/

void FreePtr(void *_ptr)
{
    free(_ptr);
}

int PrintSumIntegersAction(void *_element, void *_sumPtr)
{
    int *numberPtr = (int *)_element;
    int *sumPtr = (int *)_sumPtr;
    *sumPtr += *numberPtr;
    printf("%d  \n", *sumPtr);
    return 1;
}

int PrintIntegerAction(void *_element, void *_context)
{
    UNUSED(_context);

    int *numberPtr = (int *)_element;
    printf("%d  \n", *numberPtr);
    return 1;
}

Queue *TestQueueSetUp(size_t _capacity)
{
    Queue *queue = QueueCreate(_capacity);
    assert(queue != NULL);
    return queue;
}

void TestTearDown(Queue *_queue, DestroyItem _elementDestroy)
{
    QueueDestroy(&_queue, _elementDestroy);
    assert(_queue == NULL);
}

bool TestQueueCreate(void)
{
    printf(" * Test Queue Create\n");
    printf("Test create queue with 0 size\n");
    Queue *queue = QueueCreate(0);
    assert(queue == NULL);

    printf("Test create queue with capacity 10\n");
    queue = QueueCreate(10);
    assert(queue != NULL);

    printf("Clean up queue\n");
    QueueDestroy(&queue, NULL);
    assert(queue == NULL);

    printf("[Passed] Test Queue Create\n==\n");
    return true;
}

bool TestInsertErrorCases(void)
{
    printf(" * Test Insert Error Cases\n");

    printf("Test overflow error\n");
    Queue *queue = QueueCreate(5);
    assert(queue != NULL);

    size_t array[] = {0, 1, 2, 3, 4};
    size_t overflowItem = 5;
    for (size_t i = 0; i < 5; ++i)
    {
        assert(QueueInsert(queue, &array[i]) == QUEUE_SUCCESS);
    }

    assert(QueueInsert(queue, (void*) &overflowItem) == QUEUE_OVERFLOW_ERROR);

    printf("Test uninitialized queue\n");
    size_t someItem = 10;
    Queue *uninitializedQueue = NULL;
    assert(QueueInsert(uninitializedQueue, &someItem) == QUEUE_UNINITIALIZED_ERROR);

    printf("Test null item\n");
    void *nullItem = NULL;
    assert(QueueInsert(queue, nullItem) == QUEUE_NULL_PTR_ERROR);

    TestTearDown(queue, NULL);
    printf("[Passed] Test Insert Error Cases\n==\n");

    return true;
}



bool TestQueueRemove(void)
{
    printf(" * Test Queue Remove (FIFO)\n");
    const size_t capacity = 5;
    Queue *queue = QueueCreate(capacity);
    assert(queue != NULL);

    size_t i;
    size_t array[] = {0, 1, 2, 3, 4};
    for (i = 0; i < capacity; ++i)
    {
        assert(QueueInsert(queue, &array[i]) == QUEUE_SUCCESS);
    }

    for (size_t j = 0; j < capacity; ++j)
    {
        size_t poppedValue = 0;
        size_t *ptrPoppedValue = &poppedValue;
        assert(QueueRemove(queue, (void **)&ptrPoppedValue) == QUEUE_SUCCESS);
        assert(*ptrPoppedValue == array[j]);
    }

    TestTearDown(queue, NULL);
    printf("[Passed] Test Queue Remove\n==\n");
    return true;
}

bool TestQueueRemoveErrorCases(void)
{
    printf(" * Test Queue Remove Error Cases\n");
    Queue *queue = QueueCreate(5);
    assert(queue != NULL);

    size_t i = 0;
    size_t *ptrItem = &i;
    printf("Test uninitialized queue\n");
    Queue *uninitializedQueue = NULL;
    assert(QueueRemove(uninitializedQueue, (void **)&ptrItem) ==
           QUEUE_UNINITIALIZED_ERROR);

    printf("Test null item-pointer\n");
    void *nullItem = NULL;
    assert(QueueRemove(queue, nullItem) == QUEUE_NULL_PTR_ERROR);

    Queue *emptyQueue = TestQueueSetUp(2);

    printf("Test empty queue\n");
    assert(QueueRemove(queue, (void **)&ptrItem) == QUEUE_SIZE_ZERO_ERROR);

    TestTearDown(emptyQueue, NULL);
    TestTearDown(queue, NULL);
    printf("[Passed] Test Queue Remove Error Cases\n==\n");
    return true;
}

bool TestQueueIsEmpty(void)
{
    printf(" * Test Queue Is Empty\n");
    printf("Test empty queue\n");
    Queue *queue = TestQueueSetUp(1);

    assert(QueueIsEmpty(queue));

    printf("Test full-capacity queue\n");
    size_t x = 42;
    assert(QueueInsert(queue, &x) == QUEUE_SUCCESS);
    assert(!QueueIsEmpty(queue));

    printf("Test queue after removing all items\n");
    size_t *out = NULL;
    assert(QueueRemove(queue, (void**)&out) == QUEUE_SUCCESS);
    assert(out == &x);
    assert(QueueIsEmpty(queue));

    TestTearDown(queue, NULL);
    printf("[Passed] Test Queue Is Empty\n==\n");
    return true;
}

bool TestQueueForEach(void)
{
    printf(" * Test Queue ForEach\n");
    const size_t capacity = 10;
    Queue *queue = QueueCreate(capacity);
    assert(queue != NULL);

    /* Test foreach error cases */
    printf("Test foreach null queue\n");
    assert(QueueForEach(NULL, NULL, NULL) == 0);
    printf("Test foreach null action\n");
    assert(QueueForEach(queue, NULL, NULL) == 0);
    printf("Test foreach empty queue\n");
    assert(QueueForEach(queue, PrintIntegerAction, NULL) == 0);

    size_t half = capacity / 2;
    int array[capacity];
    for (size_t i = 0 ; i < capacity ; ++i)
    {
        array[i] = i;
    }

    for (size_t i = 0 ; i < half ; ++i)
    {
        assert(QueueInsert(queue, &array[i]) == QUEUE_SUCCESS);
    }

    printf("Test foreach half queue\n");
    assert(QueueForEach(queue, PrintIntegerAction, NULL) == 5);

    printf("Test foreach on emptied queue\n");
    size_t j = 0;
    while (!QueueIsEmpty(queue))
    {
        int *ptrItem = NULL;
        assert(QueueRemove(queue, (void**)&ptrItem) == QUEUE_SUCCESS);
        assert(*ptrItem == array[j++]);
    }

    assert(QueueForEach(queue, PrintIntegerAction, NULL) == 0);

    for (size_t i = 0 ; i < capacity; ++i)
    {
        assert(QueueInsert(queue, &array[i]) == QUEUE_SUCCESS);
    }

    printf("Test foreach full queue\n");
    assert(QueueForEach(queue, PrintIntegerAction, NULL) == capacity);


    printf("Test full queue with a null action\n");
    assert(QueueForEach(queue, NULL, NULL) == 0);

    printf("Test foreach full queue with a sum action\n");
    int sum = 0;
    int *ptrSum = &sum;
    assert(QueueForEach(queue, PrintSumIntegersAction, ptrSum) == capacity);

    // Calculate the sum 
    int realSum = 0;
    for (size_t i = 0 ; i < capacity ; ++i)
    {
        realSum += array[i];
    }
    assert(sum == realSum);

    TestTearDown(queue, NULL);
    printf("[Passed] Test Queue ForEach\n==\n");
    return true;
}

int main(void)
{
    bool (*testFunctions[])(void) = {
        TestQueueCreate,
        TestInsertErrorCases,
        TestQueueRemove,
        TestQueueRemoveErrorCases,
        TestQueueIsEmpty,
        TestQueueForEach
    };

    size_t count = 0;
    for (size_t i = 0; i < LENGTH(testFunctions); ++i)
    {
        testFunctions[i]();
        count++;
    }

    printf("============================\n");
    printf("Passed %zu out of %zu tests\n", count, LENGTH(testFunctions));

    return 0;
}
